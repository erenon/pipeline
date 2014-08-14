/**
 * Boost.Pipeline
 *
 * Copyright 2014 Benedek Thaler
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See $PIPELINE_WEBSITE$ for documentation
 */

#ifndef BOOST_PIPELINE_DETAIL_SEGMENT_HPP
#define BOOST_PIPELINE_DETAIL_SEGMENT_HPP

#include <iterator>
#include <functional>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <memory>

#include <boost/pipeline/queue.hpp>
#include <boost/pipeline/execution.hpp>
#include <boost/pipeline/threading.hpp>
#include <boost/pipeline/type_erasure.hpp>
#include <boost/pipeline/detail/task.hpp>


namespace boost {
namespace pipeline {
namespace detail {

/**
 * Represents a series of connected operations.
 *
 * Template arguments:
 *
 *  - Parent: type of the segment before this segment in the pipline;
 *    in case of (foo | bar | baz) baz has a type of
 *    segment<segment<segment<?, Out>, Out> Out>
 *
 *    Parent must have a run<OutIt>(OutIt) method and value_type typedef.
 *    The method run feeds instances of value_type into a range
 *    pointed by the given iterator.
 *
 *  - Output: type of emitted entries
 */
template <typename Parent, typename Output>
class basic_segment :
  public segment_concept<
    typename std::remove_reference<
      typename std::remove_reference<Parent>::type::root_type
    >::type,
    Output
  >
{
public:
  typedef typename std::remove_reference<
    typename std::remove_reference<Parent>::type::value_type
  >::type input_type;

  typedef typename std::remove_reference<
    typename std::remove_reference<Parent>::type::root_type
  >::type root_type;

  typedef Output value_type;

  /**
   * Creates a new segment by concatenating `function` after `parent`.
   *
   * Parent and function_type must be copy constructible
   */
  basic_segment(const Parent& parent)
    :_parent(parent)
  {}

  /**
   * Runs the segment.
   *
   * Gets the upstream queues front of the `_parent` segment,
   * transforms each item using `_function`
   * and feeds them into the downstream queue accessed through `target`.
   */
  template <typename Task, typename Function>
  void run(thread_pool& pool, Function& function, const queue_back<value_type>& target)
  {
    Task task(function, target);
    _parent.run(pool, task.get_queue_back());

    pool.submit(std::move(task));
  }

  void connect_to(runnable_concept<root_type>& parent)
  {
    _parent.connect_to(parent);
  }

protected:
  Parent _parent;          /**< parent segment, provider of input */
};

template <typename Parent, typename Output, bool IsSink = std::is_same<Output, void>::value>
class one_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef std::function<value_type(const input_type&)> function_type;

  typedef one_one_task<input_type, value_type, function_type> task_type;

  one_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /** @copydoc basic_segment::run */
  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    base_segment::template run<task_type>(pool, _function, target);
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new one_one_segment<Parent, Output, IsSink>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output>
class one_one_segment<Parent, Output, true> : public basic_segment<Parent, terminated>
{
  typedef basic_segment<Parent, terminated> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef terminated value_type;

  typedef std::function<Output(const input_type&)> function_type;

  typedef single_consume_output_task<input_type, function_type> task_type;

  one_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  execution run(thread_pool& pool)
  {
    std::promise<void> promise;
    auto future = promise.get_future();

    task_type task(std::move(promise), _function);

    base_segment::_parent.run(pool, task.get_queue_back());

    pool.submit(std::move(task));

    return execution(std::move(future));
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new one_one_segment<Parent, Output, true>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class one_n_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef std::function<R(
    const input_type&,
    queue_back<Output>&
  )> function_type;

  typedef one_n_task<input_type, value_type, function_type> task_type;

  one_n_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /** @copydoc basic_segment::run */
  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    base_segment::template run<task_type>(pool, _function, target);
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new one_n_segment<Parent, Output, R>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, bool IsSink = std::is_same<Output, void>::value>
class n_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef std::function<Output(
    queue_front<input_type>&
  )> function_type;

  typedef n_one_task<input_type, value_type, function_type> task_type;

  n_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /** @copydoc basic_segment::run */
  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    base_segment::template run<task_type>(pool, _function, target);
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new n_one_segment<Parent, Output, IsSink>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output>
class n_one_segment<Parent, Output, true> : public basic_segment<Parent, void>
{
  typedef basic_segment<Parent, void> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef void value_type;

  typedef std::function<Output(
    queue_front<input_type>&
  )> function_type;

  typedef multi_consume_output_task<input_type, function_type> task_type;

  n_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  execution run(thread_pool& pool)
  {
    std::promise<void> promise;
    auto future = promise.get_future();

    task_type task(std::move(promise), _function);

    base_segment::_parent.run(pool, task.get_queue_back());

    pool.submit(std::move(task));

    return execution(std::move(future));
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new n_one_segment<Parent, Output, true>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class n_m_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef std::function<R(
    queue_front<input_type>&,
    queue_back<Output>&
  )> function_type;

  typedef n_m_task<input_type, value_type, function_type> task_type;

  n_m_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /** @copydoc basic_segment::run */
  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    base_segment::template run<task_type>(pool, _function, target);
  }

  std::unique_ptr<segment_concept<root_type, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, value_type>>(
      new n_m_segment<Parent, Output, R>(*this)
    ));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Iterator>
class range_input_segment
  : public segment_concept<terminated, typename std::remove_reference<decltype(*std::declval<Iterator>())>::type>
{
public:
  typedef void root_type;
  typedef typename std::remove_reference<decltype(*std::declval<Iterator>())>::type value_type;

  typedef range_input_task<Iterator, value_type> task_type;

  range_input_segment(const Iterator& begin, const Iterator& end)
    :_begin(begin),
     _end(end)
  {}

  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    task_type task(_begin, _end, target);
    pool.submit(std::move(task));
  }

  std::unique_ptr<segment_concept<terminated, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<terminated, value_type>>(
      new range_input_segment<Iterator>(*this)
    ));
  }

private:
  const Iterator _begin;
  const Iterator _end;
};

template <typename Output>
class queue_input_segment : public segment_concept<terminated, Output>
{
public:
  typedef void root_type;
  typedef Output value_type;

  typedef queue_input_task<Output> task_type;

  queue_input_segment(queue<Output>& queue)
    :_queue(queue)
  {}

  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    task_type task(_queue, target);
    pool.submit(std::move(task));
  }

  std::unique_ptr<segment_concept<terminated, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<terminated, value_type>>(
      new queue_input_segment<Output>(*this)
    ));
  }

private:
  queue<Output>& _queue;
};

template <typename Callable, typename Output>
class generator_input_segment : public segment_concept<terminated, Output>
{
public:
  typedef void root_type;
  typedef Output value_type;
  typedef Callable function_type;

  typedef generator_input_task<Callable, Output> task_type;

  generator_input_segment(const function_type& generator)
    :_generator(generator)
  {}

  void run(thread_pool& pool, const queue_back<value_type>& target)
  {
    task_type task(_generator, target);
    pool.submit(std::move(task));
  }

  std::unique_ptr<segment_concept<terminated, value_type>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<terminated, value_type>>(
      new generator_input_segment<Callable, Output>(*this)
    ));
  }

private:
  function_type _generator;
};

template <typename Parent, typename Container>
class range_output_segment : public basic_segment<Parent, terminated>
{
  typedef basic_segment<Parent, terminated> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef range_output_task<input_type, Container> task_type;

  range_output_segment(
    const Parent& parent,
    Container& container
  )
    :base_segment(parent),
     _container(container)
  {}

  execution run(thread_pool& pool)
  {
    std::promise<void> promise;
    auto out_it = std::back_inserter(_container);

    auto future = promise.get_future();

    task_type task(std::move(promise), out_it);

    base_segment::_parent.run(pool, task.get_queue_back());

    pool.submit(std::move(task));

    return execution(std::move(future));
  }

  std::unique_ptr<segment_concept<root_type, terminated>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, terminated>>(
      new range_output_segment<Parent, Container>(*this)
    ));
  }

private:
  Container& _container;
};

template <typename Parent>
class queue_output_segment : public basic_segment<Parent, terminated>
{
  typedef basic_segment<Parent, terminated> base_segment;

public:
  typedef typename base_segment::root_type  root_type;
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef queue_output_task<input_type> task_type;

  queue_output_segment(
    const Parent& parent,
    queue<input_type>& queue
  )
    :base_segment(parent),
     _queue(queue)
  {}

  execution run(thread_pool& pool)
  {
    queue_back<input_type> parent_downstream(_queue);
    base_segment::_parent.run(pool, parent_downstream);

    std::promise<void> promise;
    auto future = promise.get_future();

    task_type task(std::move(promise), _queue);
    pool.submit(std::move(task));

    return execution(std::move(future));
  }

  std::unique_ptr<segment_concept<root_type, terminated>> clone() const
  {
    return std::move(std::unique_ptr<segment_concept<root_type, terminated>>(
      new queue_output_segment<Parent>(*this)
    ));
  }

private:
  queue<input_type>& _queue;
};

//
// is_connectable_segment predicate
//

template <typename NotSegment>
struct is_connectable_segment : public std::false_type {};

template <typename P, typename O>
struct is_connectable_segment<basic_segment<P, O>> : public std::true_type {};

template <typename P>
struct is_connectable_segment<basic_segment<P, void>> : public std::false_type {};

template <typename P, typename O>
struct is_connectable_segment<one_one_segment<P, O>> : public std::true_type {};

template <typename P>
struct is_connectable_segment<one_one_segment<P, void>> : public std::false_type {};

template <typename P, typename O, typename R>
struct is_connectable_segment<one_n_segment<P, O, R>> : public std::true_type {};

template <typename P, typename O>
struct is_connectable_segment<n_one_segment<P, O>> : public std::true_type {};

template <typename P>
struct is_connectable_segment<n_one_segment<P, void>> : public std::false_type {};

template <typename P, typename O, typename R>
struct is_connectable_segment<n_m_segment<P, O, R>> : public std::true_type {};

template <typename I>
struct is_connectable_segment<range_input_segment<I>> : public std::true_type {};

template <typename T>
struct is_connectable_segment<queue_input_segment<T>> : public std::true_type {};

template <typename C, typename O>
struct is_connectable_segment<generator_input_segment<C, O>> : public std::true_type {};

//
// to_sink_segment
//

template <typename NotSinkableSegment>
struct to_sink_segment {};

template <typename P, typename O, bool S>
struct to_sink_segment<one_one_segment<P, O, S>>
{
  typedef one_one_segment<P, O, true> type;
};

template <typename P, typename O, bool S>
struct to_sink_segment<n_one_segment<P, O, S>>
{
  typedef n_one_segment<P, O, true> type;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_SEGMENT_HPP
