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
class basic_segment
{
public:
  typedef typename std::remove_reference<
    typename std::remove_reference<Parent>::type::value_type
  >::type input_type;

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
   * and feeds them into the downstream queue.
   */
  template <typename Task, typename Function>
  queue_front<value_type> run(thread_pool& pool, Function& function)
  {
    auto qf = _parent.run(pool);
    auto downstream_ptr = std::make_shared<queue<value_type>>();
    queue_back<value_type> qb(downstream_ptr);

    Task task(qf, qb, function);

    pool.submit(task);

    return downstream_ptr;
  }

protected:
  Parent _parent;          /**< parent segment, provider of input */
};

template <typename Parent, typename Output>
class one_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
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
  queue_front<value_type> run(thread_pool& pool)
  {
    return base_segment::template run<task_type>(pool, _function);
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent>
class one_one_segment<Parent, void> : public basic_segment<Parent, void>
{
  typedef basic_segment<Parent, void> base_segment;

public:
  typedef typename base_segment::input_type input_type;
  typedef void value_type;

  typedef std::function<void(const input_type&)> function_type;

  typedef one_sink_task<input_type, function_type> task_type;

  one_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  execution run(thread_pool& pool)
  {
    auto promise_ptr = std::make_shared<std::promise<bool>>();
    auto future = promise_ptr->get_future();
    auto queue_front = base_segment::_parent.run(pool);

    task_type task(promise_ptr, queue_front, _function);
    pool.submit(task);

    return execution(std::move(future));
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class one_n_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
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
  queue_front<value_type> run(thread_pool& pool)
  {
    return base_segment::template run<task_type>(pool, _function);
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output>
class n_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::input_type input_type;
  typedef typename base_segment::value_type value_type;

  typedef std::function<Output(
    queue_front<Output>&
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
  queue_front<value_type> run(thread_pool& pool)
  {
    return base_segment::template run<task_type>(pool, _function);
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class n_m_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
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
  queue_front<value_type> run(thread_pool& pool)
  {
    return base_segment::template run<task_type>(pool, _function);
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Iterator>
class range_input_segment
{
public:
  typedef typename std::remove_reference<decltype(*std::declval<Iterator>())>::type value_type;

  range_input_segment(const Iterator& begin, const Iterator& end)
    :_current(begin),
     _end(end)
  {}

  queue_front<value_type> run(thread_pool& pool)
  {
    auto queuePtr = std::make_shared<queue<value_type>>();
    queue_front<value_type> qf(queuePtr);

    auto task = [this, queuePtr] () -> bool
    {
      auto& q(*queuePtr);

      while (_current != _end)
      {
        auto status = q.try_push(*_current);
        if (status == queue_op_status::SUCCESS)
        {
          ++_current;
        }
        else
        {
          return false; // not finished
        }
      }

      q.close();

      return true;
    };

    pool.submit(task);

    return qf;
  }

private:
  Iterator _current;
  const Iterator _end;
};

template <typename Callable, typename Output>
class generator_input_segment
{
public:
  typedef Output value_type;
  typedef Callable function_type;

  generator_input_segment(const function_type& generator)
    :_generator(generator)
  {}

  queue_front<value_type> run(thread_pool& pool)
  {
    auto queuePtr = std::make_shared<queue<value_type>>();
    queue_back<value_type> qb(queuePtr);
    queue_front<value_type> qf(queuePtr);

    auto task = [this, qb] () mutable -> bool
    {
      _generator(qb);

      qb.close();

      return true;
    };

    pool.submit(task);

    return qf;
  }

private:
  function_type _generator;
};

template <typename Container, typename Parent> // TODO change Arg order
class range_output_segment: public basic_segment<Parent, void>
{
  typedef basic_segment<Parent, void> base_segment;

public:
  typedef typename base_segment::input_type input_type;

  range_output_segment(
    const Parent& parent,
    Container& container
  )
    :base_segment(parent),
     _container(container)
  {}

  execution run(thread_pool& pool)
  {
    auto promise_ptr = std::make_shared<std::promise<bool>>();
    auto future = promise_ptr->get_future();
    auto queue_front = base_segment::_parent.run(pool);
    auto out_it = std::back_inserter(_container);

    range_output_task<input_type, Container> task(promise_ptr, queue_front, out_it);
    pool.submit(task);

    return execution(std::move(future));
  }

private:
  Container& _container;
};

//
// is_segment predicate
// TODO rename to is_connectable_segment,
// since *_output_segment is segment but not connectable
// aslo segments with Output = void are not connectable
//

template <typename NotSegment>
struct is_segment : public std::false_type {};

template <typename P, typename O>
struct is_segment<basic_segment<P, O>> : public std::true_type {};

template <typename P, typename O>
struct is_segment<one_one_segment<P, O>> : public std::true_type {};

template <typename P, typename O, typename R>
struct is_segment<one_n_segment<P, O, R>> : public std::true_type {};

template <typename P, typename O>
struct is_segment<n_one_segment<P, O>> : public std::true_type {};

template <typename P, typename O, typename R>
struct is_segment<n_m_segment<P, O, R>> : public std::true_type {};

template <typename I>
struct is_segment<range_input_segment<I>> : public std::true_type {};

template <typename C, typename O>
struct is_segment<generator_input_segment<C, O>> : public std::true_type {};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_SEGMENT_HPP
