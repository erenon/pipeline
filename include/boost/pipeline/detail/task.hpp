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

#ifndef BOOST_PIPELINE_DETAIL_TASK_HPP
#define BOOST_PIPELINE_DETAIL_TASK_HPP

namespace boost {
namespace pipeline {
namespace detail {

template <typename Input, typename Output, typename Transformation>
class one_one_task
{
public:
  one_one_task(
    const queue_front<Input>& queue_front,
    const queue_back<Output>& queue_back,
    const Transformation& function
  )
    :_queue_front(queue_front),
     _queue_back(queue_back),
     _transformation(function)
  {}

  bool operator()()
  {
    while (true)
    {
      if ( ! _queue_front.is_empty())
      {
        const auto& input = _queue_front.front();
        const auto output = _transformation(input);
        auto status = _queue_back.try_push(output);

        if (status == queue_op_status::SUCCESS)
        {
          _queue_front.try_pop();
        }
        else if (status == queue_op_status::FULL)
        {
          // downstream queue is full, yield
          return false;
        }
      }
      else // upstream queue is empty
      {
        if (_queue_front.is_closed())
        {
          _queue_back.close();
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
  queue_front<Input> _queue_front;
  queue_back<Output> _queue_back;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class one_n_task
{
public:
  one_n_task(
    const queue_front<Input>& queue_front,
    const queue_back<Output>& queue_back,
    const Transformation& function
  )
    :_queue_front(queue_front),
     _queue_back(queue_back),
     _transformation(function)
  {}

  bool operator()()
  {
    while (true)
    {
      if (_queue_front.is_closed() && _queue_front.is_empty())
      {
        _queue_back.close();
        return true; // task finished
      }

      if ( ! _queue_front.is_empty() && ! _queue_back.is_full() )
      {
        const auto& input = _queue_front.front();
        _transformation(input, _queue_back);
        _queue_front.try_pop();
      }
      else
      {
        return false; // yield
      }
    }
  }

private:
  queue_front<Input> _queue_front;
  queue_back<Output> _queue_back;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class n_one_task
{
public:
  n_one_task(
    const queue_front<Input>& queue_front,
    const queue_back<Output>& queue_back,
    const Transformation& function
  )
    :_queue_front(queue_front),
     _queue_back(queue_back),
     _transformation(function)
  {}

  bool operator()()
  {
    while (true)
    {
      // try buffer
      if (_has_buffered)
      {
        auto status = _queue_back.try_push(_buffer);
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

      auto output = _transformation(_queue_front);
      auto status = _queue_back.try_push(output);
      if (status == queue_op_status::FULL)
      {
        // downstream queue is full, buffer output
        _has_buffered = true;
        _buffer = output;
        return false; // yield
      }

      if (_queue_front.is_closed() && _queue_front.is_empty())
      {
        _queue_back.close();
        return true;
      }
    }
  }

private:
  queue_front<Input> _queue_front;
  queue_back<Output> _queue_back;
  Transformation _transformation;
  bool _has_buffered = false;
  Output _buffer;
};

template <typename Input, typename Output, typename Transformation>
class n_m_task
{
public:

  n_m_task(
    const queue_front<Input>& queue_front,
    const queue_back<Output>& queue_back,
    const Transformation& function
  )
    :_queue_front(queue_front),
     _queue_back(queue_back),
     _transformation(function)
  {}

  bool operator()()
  {
    while (_queue_front.read_available())
    {
      _transformation(_queue_front, _queue_back);
    }

    if (_queue_front.is_closed() == false)
    {
      return false; // yield
    }

    _queue_back.close();
    return true;
  }

private:
  queue_front<Input> _queue_front;
  queue_back<Output> _queue_back;
  Transformation _transformation;
};

template <typename Input, typename Container>
class range_output_task
{
public:
  range_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& queue_front,
    const std::back_insert_iterator<Container>& out_it
  )
    :_promise_ptr(promise_ptr),
     _queue_front(queue_front),
     _out_it(out_it)
  {}

  bool operator()()
  {
    while (true)
    {
      Input entry;

      auto status = _queue_front.try_pop(entry);
      if (status == queue_op_status::SUCCESS)
      {
        *_out_it = entry;
      }
      else if (status == queue_op_status::CLOSED) // only if queue is empty
      {
        _promise_ptr->set_value(true);
        return true;
      }
      else // queue was empty but not closed, more entries may arrive
      {
        return false; // not finished
      }
    }
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _queue_front;
  std::back_insert_iterator<Container> _out_it;
};

template <typename Input, typename Consumer>
class single_consume_output_task
{
public:
  single_consume_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& queue_front,
    const Consumer& consumer
  )
    :_promise_ptr(promise_ptr),
     _queue_front(queue_front),
     _consumer(consumer)
  {}

  bool operator()()
  {
    while (true)
    {
      Input entry;

      auto status = _queue_front.try_pop(entry);
      if (status == queue_op_status::SUCCESS)
      {
        _consumer(entry);
      }
      else if (status == queue_op_status::CLOSED) // only if queue is empty
      {
        _promise_ptr->set_value(true);
        return true;
      }
      else // queue was empty but not closed, more entries may arrive
      {
        return false; // not finished
      }
    }
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _queue_front;
  Consumer _consumer;
};

template <typename Input, typename Consumer>
class multi_consume_output_task
{
public:
  multi_consume_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& queue_front,
    const Consumer& consumer
  )
    :_promise_ptr(promise_ptr),
     _queue_front(queue_front),
     _consumer(consumer)
  {}

  bool operator()()
  {
    while (true)
    {
      if (_queue_front.is_empty())
      {
        // finish if closed, yield if not
        if (_queue_front.is_closed())
        {
          _promise_ptr->set_value(true);
          return true;
        }
        else
        {
          return false;
        }
      }

      _consumer(_queue_front);
    }
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _queue_front;
  Consumer _consumer;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_TASK_HPP
