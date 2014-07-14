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
    const queue_front<Input>& upstream,
    const queue_back<Output>& downstream,
    const Transformation& function
  )
    :_upstream(upstream),
     _downstream(downstream),
     _transformation(function)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      auto input = _upstream.pull();
      auto output = _transformation(input);
      _downstream.push(output);
    }

    _downstream.close();
  }

private:
  queue_front<Input> _upstream;
  queue_back<Output> _downstream;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class one_n_task
{
public:
  one_n_task(
    const queue_front<Input>& upstream,
    const queue_back<Output>& downstream,
    const Transformation& function
  )
    :_upstream(upstream),
     _downstream(downstream),
     _transformation(function)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      auto input = _upstream.pull();
      _transformation(input, _downstream);
    }

    _downstream.close();
  }

private:
  queue_front<Input> _upstream;
  queue_back<Output> _downstream;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class n_one_task
{
public:
  n_one_task(
    const queue_front<Input>& upstream,
    const queue_back<Output>& downstream,
    const Transformation& function
  )
    :_upstream(upstream),
     _downstream(downstream),
     _transformation(function)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      auto output = _transformation(_upstream);
      _downstream.push(std::move(output));
    }

    _downstream.close();
  }

private:
  queue_front<Input> _upstream;
  queue_back<Output> _downstream;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class n_m_task
{
public:

  n_m_task(
    const queue_front<Input>& upstream,
    const queue_back<Output>& downstream,
    const Transformation& function
  )
    :_upstream(upstream),
     _downstream(downstream),
     _transformation(function)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      _transformation(_upstream, _downstream);
    }

    _downstream.close();
  }

private:
  queue_front<Input> _upstream;
  queue_back<Output> _downstream;
  Transformation _transformation;
};

template <typename Input, typename Container>
class range_output_task
{
public:
  range_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& upstream,
    const std::back_insert_iterator<Container>& out_it
  )
    :_promise_ptr(promise_ptr),
     _upstream(upstream),
     _out_it(out_it)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      auto input = _upstream.pull();
      *_out_it = input;
    }

    _promise_ptr->set_value(true);
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _upstream;
  std::back_insert_iterator<Container> _out_it;
};

template <typename Input, typename Consumer>
class single_consume_output_task
{
public:
  single_consume_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& upstream,
    const Consumer& consumer
  )
    :_promise_ptr(promise_ptr),
     _upstream(upstream),
     _consumer(consumer)
  {}

  void operator()()
  {
    while ( ! _upstream.is_closed() || ! _upstream.is_empty())
    {
      _consumer(_upstream.pull());
    }

    _promise_ptr->set_value(true);
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _upstream;
  Consumer _consumer;
};

template <typename Input, typename Consumer>
class multi_consume_output_task
{
public:
  multi_consume_output_task(
    const std::shared_ptr<std::promise<bool>>& promise_ptr,
    const queue_front<Input>& upstream,
    const Consumer& consumer
  )
    :_promise_ptr(promise_ptr),
     _upstream(upstream),
     _consumer(consumer)
  {}

  void operator()()
  {
    while (true)
    {
      while ( ! _upstream.is_closed() || ! _upstream.is_empty())
      {
        _consumer(_upstream);
      }

      _promise_ptr->set_value(true);
    }
  }

private:
  std::shared_ptr<std::promise<bool>> _promise_ptr;
  queue_front<Input> _upstream;
  Consumer _consumer;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_TASK_HPP
