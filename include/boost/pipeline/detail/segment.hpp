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
#include <boost/pipeline/detail/log.hpp>

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

protected:
  Parent _parent;          /**< parent segment, provider of input */
};

template <typename Parent, typename Output>
class one_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<value_type(const input_type&)> function_type;

  one_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * transforms each item using `_function`
   * and feeds them into the returned queue.
   */
  queue_back<value_type> run(thread_pool& pool)
  {
    auto qb = base_segment::_parent.run(pool);
    auto downstream_ptr = std::make_shared<queue<value_type>>();
    queue_front<value_type> qf(downstream_ptr);

    Task task(qb, qf, _function);

    pool.submit(task);

    return downstream_ptr;
  }

private:
  class Task
  {
  public:
    typedef typename Parent::value_type in_entry;

    Task(
      const queue_back<in_entry>& queue_back,
      const queue_front<value_type>& queue_front,
      const function_type& function
    )
      :_queue_back(queue_back),
       _queue_front(queue_front),
       _function(function)
    {}

    bool operator()()
    {
      while (true)
      {
        if ( ! _queue_back.is_empty())
        {
          const auto& input = _queue_back.front();
          const auto output = _function(input);
          auto status = _queue_front.try_push(output);

          if (status == queue_op_status::SUCCESS)
          {
            _queue_back.try_pop();
          }
          else if (status == queue_op_status::FULL)
          {
            // downstream queue is full, yield
            return false;
          }
        }
        else // upstream queue is empty
        {
          if (_queue_back.is_closed())
          {
            _queue_front.close();
            return true; // task finished
          }
          else
          {
            return false; // yield
          }
        }
      }
    }

  private:
    queue_back<in_entry> _queue_back;
    queue_front<value_type> _queue_front;
    function_type _function;
  };

  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class one_n_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<R(
    const input_type&,
    queue_front<Output>&
  )> function_type;

  one_n_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * transforms each item using `_function`.
   *
   * Also passes the soon to be returned
   * queue to `_function`, it's the transformations
   * responsibility to feed the queue at will.
   */
  queue_back<Output> run(thread_pool& pool)
  {
    auto qb = base_segment::_parent.run(pool);
    auto downstream_ptr = std::make_shared<queue<value_type>>();
    queue_front<value_type> qf(downstream_ptr);

    Task task(qb, qf, _function);

    pool.submit(task);

    return downstream_ptr;
  }

private:
  class Task
  {
    typedef typename Parent::value_type in_entry;

  public:
    Task(
      const queue_back<in_entry>& queue_back,
      const queue_front<value_type>& queue_front,
      const function_type& function
    )
      :_queue_back(queue_back),
       _queue_front(queue_front),
       _function(function)
    {}

    bool operator()()
    {
      while (true)
      {
        if (_queue_back.is_closed() && _queue_back.is_empty())
        {
          _queue_front.close();
          return true; // task finished
        }

        if ( ! _queue_back.is_empty() && ! _queue_front.is_full() )
        {
          const auto& input = _queue_back.front();
          _function(input, _queue_front);
          _queue_back.try_pop();
        }
        else
        {
          return false; // yield
        }
      }
    }

  private:
    queue_back<in_entry> _queue_back;
    queue_front<value_type> _queue_front;
    function_type _function;
  };

  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output>
class n_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<Output(
    queue_back<Output>&
  )> function_type;

  n_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * and runs `_function` until queue is not closed.
   */
  queue_back<Output> run(thread_pool& pool)
  {
    auto qb = base_segment::_parent.run(pool);
    auto downstream_ptr = std::make_shared<queue<value_type>>();
    queue_front<value_type> qf(downstream_ptr);

    Task task(qb, qf, _function);

    pool.submit(task);

    return downstream_ptr;
  }

private:
  class Task
  {
  public:
    typedef typename Parent::value_type in_entry;

    Task(
      const queue_back<in_entry>& queue_back,
      const queue_front<value_type>& queue_front,
      const function_type& function
    )
      :_queue_back(queue_back),
       _queue_front(queue_front),
       _function(function)
    {}

    bool operator()()
    {
      while (true)
      {
        // try buffer
        if (_has_buffered)
        {
          auto status = _queue_front.try_push(_buffer);
          if (status == queue_op_status::SUCCESS)
          {
            _has_buffered = false;
          }
          else
          {
            // downstream queue is still full
            return false; // yield
          }
        }

        auto output = _function(_queue_back);
        auto status = _queue_front.try_push(_buffer);
        if (status == queue_op_status::FULL)
        {
          // downstream queue is full, buffer output
          _has_buffered = true;
          _buffer = output;
          return false; // yield
        }

        if (_queue_back.is_closed() && _queue_back.is_empty())
        {
          _queue_front.close();
          return true;
        }
      }
    }

  private:
    queue_back<in_entry> _queue_back;
    queue_front<value_type> _queue_front;
    function_type _function;
    bool _has_buffered = false;
    value_type _buffer;
  };

  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class n_m_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<R(
    queue_back<input_type>&,
    queue_front<Output>&
  )> function_type;

  n_m_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * and passes the parent queue and the
   * soon to be returned queue to `_function`.
   * It's the transformations responsibility
   * to feed the queue at will.
   */
  queue_back<Output> run()
  {
    auto in_queue = base_segment::_parent.run();
    queue<Output> out_queue;

    _function(in_queue, out_queue);

    return out_queue;
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Iterator>
class input_segment
{
public:
  typedef typename std::remove_reference<decltype(*std::declval<Iterator>())>::type value_type;

  input_segment(const Iterator& begin, const Iterator& end)
    :_current(begin),
     _end(end)
  {}

  queue_back<value_type> run(thread_pool& pool)
  {
    auto queuePtr = std::make_shared<queue<value_type>>();
    queue_back<value_type> qb(queuePtr);

    auto task = [this, queuePtr] () -> bool
    {
      auto& q(*queuePtr);

      while (_current != _end)
      {
//        LOG("[PROD] Try push: %d", *_current);
        auto status = q.try_push(*_current);
        if (status == queue_op_status::SUCCESS)
        {
//          LOG("[PROD] Push Success : %d", *_current);
          ++_current;
        }
        else
        {
//          LOG("[PROD] Push Failure : %d", *_current);
          return false; // not finished
        }
      }

//      LOG0("[PROD] Closing queue");
      q.close();

      return true;
    };

    pool.submit(task);

    return qb;
  }

private:
  Iterator _current;
  const Iterator _end;
};

template <typename Container, typename Parent>
class output_segment: public basic_segment<Parent, void>
{
  typedef basic_segment<Parent, void> base_segment;

public:
  output_segment(
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
    auto queue_back = base_segment::_parent.run(pool);
    auto out_it = std::back_inserter(_container);

    Task task(promise_ptr, queue_back, out_it);
    pool.submit(task);

    return execution(std::move(future));
  }

private:
  class Task
  {
  public:
    typedef queue_back<typename Parent::value_type> queue_back_t;

    Task(
      const std::shared_ptr<std::promise<bool>>& promise_ptr,
      const queue_back_t& queue_back,
      const std::back_insert_iterator<Container>& out_it
    )
      :_promise_ptr(promise_ptr),
       _queue_back(queue_back),
       _out_it(out_it)
    {}

    bool operator()()
    {
      typedef typename queue_back_t::value_type entry_t;

      while (true)
      {
        entry_t entry;

//        LOG0("[CONS] Try pop");
        auto status = _queue_back.try_pop(entry);
        if (status == queue_op_status::SUCCESS)
        {
//          LOG("[CONS] Entry popped: %d", entry);
          *_out_it = entry;
        }
        else if (status == queue_op_status::CLOSED) // only if queue is empty
        {
//          LOG0("[CONS] Stop, queue closed");
          _promise_ptr->set_value(true);
          return true;
        }
        else // queue was empty but not closed, more entries may arrive
        {
//          LOG0("[CONS] Yield, queue empty");
          return false; // not finished
        }
      }
    }

  private:
    std::shared_ptr<std::promise<bool>> _promise_ptr;
    queue_back_t _queue_back;
    std::back_insert_iterator<Container> _out_it;
  };

  Container& _container;
};

//
// is_segment predicate
// TODO rename to is_connectable_segment,
// since output_segment is segment but not connectable
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
struct is_segment<input_segment<I>> : public std::true_type {};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_SEGMENT_HPP
