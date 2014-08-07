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

#include <memory>

#include <boost/core/null_deleter.hpp>

#include <boost/pipeline/queue.hpp>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Input, typename Output, typename Transformation>
class basic_task
{
public:
  queue_back<Input> get_queue_back()
  {
    return queue_back<Input>(_input.get());
  }

protected:
  basic_task(
    const Transformation& function,
    const queue_back<Output>& downstream
  )
    :_input(new queue<Input>()),
     _downstream(downstream),
     _transformation(function)
  {}

  std::unique_ptr<queue<Input>> _input;
  queue_back<Output> _downstream;
  Transformation _transformation;
};

template <typename Input, typename Output, typename Transformation>
class one_one_task : basic_task<Input, Output, Transformation>
{
  typedef basic_task<Input, Output, Transformation> base;

public:
  one_one_task(
    const Transformation& function,
    const queue_back<Output>& downstream
  )
    :base(function, downstream)
  {}

  void operator()()
  {
    queue_front<Input> upstream(base::_input.get());

    Input input;
    while (upstream.wait_pull(input))
    {
      auto output = base::_transformation(std::move(input));
      base::_downstream.push(std::move(output));
    }

    base::_downstream.close();
  }

  using base::get_queue_back;
};

template <typename Input, typename Output, typename Transformation>
class one_n_task
  : basic_task<Input, Output, Transformation>
{
  typedef basic_task<Input, Output, Transformation> base;

public:
  one_n_task(
    const Transformation& function,
    const queue_back<Output>& downstream
  )
    :base(function, downstream)
  {}

  void operator()()
  {
    queue_front<Input> upstream(base::_input.get());

    Input input;
    while (upstream.wait_pull(input))
    {
      base::_transformation(std::move(input), base::_downstream);
    }

    base::_downstream.close();
  }

  using base::get_queue_back;
};

template <typename Input, typename Output, typename Transformation>
class n_one_task
  : basic_task<Input, Output, Transformation>
{
  typedef basic_task<Input, Output, Transformation> base;

public:
  n_one_task(
    const Transformation& function,
    const queue_back<Output>& downstream
  )
    :base(function, downstream)
  {}

  void operator()()
  {
    queue_front<Input> upstream(base::_input.get());

    while (! upstream.is_empty() || ! upstream.is_closed())
    {
      auto output = base::_transformation(upstream);
      base::_downstream.push(std::move(output));
    }

    base::_downstream.close();
  }

  using base::get_queue_back;
};

template <typename Input, typename Output, typename Transformation>
class n_m_task
  : basic_task<Input, Output, Transformation>
{
  typedef basic_task<Input, Output, Transformation> base;

public:
  n_m_task(
    const Transformation& function,
    const queue_back<Output>& downstream
  )
    :base(function, downstream)
  {}

  void operator()()
  {
    queue_front<Input> upstream(base::_input.get());

    while (! upstream.is_empty() || ! upstream.is_closed())
    {
      base::_transformation(upstream, base::_downstream);
    }

    base::_downstream.close();
  }

  using base::get_queue_back;
};

template <typename Iterator, typename Output>
class range_input_task
{
public:
  range_input_task(
    const Iterator& begin,
    const Iterator& end,
    const queue_back<Output>& downstream
  )
    :_current(begin),
     _end(end),
     _downstream(downstream)
  {}

  void operator()()
  {
    while (_current != _end)
    {
      _downstream.push(*_current);
      ++_current;
    }

    _downstream.close();
  }

private:
  Iterator _current;
  const Iterator _end;
  queue_back<Output> _downstream;
};

template <typename Output>
class queue_input_task
{
public:
  queue_input_task(
    queue<Output>& queue,
    const queue_back<Output>& downstream
  )
    :_queue(queue),
     _downstream(downstream)
  {}

  void operator()()
  {
    queue_front<Output> upstream(&_queue);

    Output output;
    while (upstream.wait_pull(output))
    {
      _downstream.push(std::move(output));
    }

    _downstream.close();
  }

private:
  queue<Output>& _queue;
  queue_back<Output> _downstream;
};

template <typename Callable, typename Output>
class generator_input_task
{
public:
  generator_input_task(
    const Callable& generator,
    const queue_back<Output>& downstream
  )
    :_generator(generator),
     _downstream(downstream)
  {}

  void operator()()
  {
    _generator(_downstream);
    _downstream.close();
  }

private:
  Callable _generator;
  queue_back<Output> _downstream;
};

template <typename Input, typename Container>
class range_output_task
{
public:
  range_output_task(
    std::promise<void>&& promise,
    const std::back_insert_iterator<Container>& out_it
  )
    :_promise(std::move(promise)),
     _input(new queue<Input>()),
     _out_it(out_it)
  {}

  void operator()()
  {
    queue_front<Input> upstream(_input.get());

    Input input;
    while (upstream.wait_pull(input))
    {
      *_out_it = std::move(input);
    }

    _promise.set_value();
  }

  queue_back<Input> get_queue_back()
  {
    return queue_back<Input>(_input.get());
  }

private:
  std::promise<void> _promise;
  std::unique_ptr<queue<Input>> _input;
  std::back_insert_iterator<Container> _out_it;
};

template <typename Input>
class queue_output_task
{
public:
  queue_output_task(
    std::promise<void>&& promise,
    queue<Input>& queue
  )
    :_promise(std::move(promise)),
     _upstream(&queue)
  {}

  void operator()()
  {
    // TODO queue_output_task is very inefficient
    while ( ! _upstream.is_closed())
    {
      std::this_thread::yield();
    }

    _promise.set_value();
  }

private:
  std::promise<void> _promise;
  queue_front<Input> _upstream;
};

template <typename Input, typename Consumer>
class single_consume_output_task
{
public:
  single_consume_output_task(
    std::promise<void>&& promise,
    const Consumer& consumer
  )
    :_promise(std::move(promise)),
     _input(new queue<Input>()),
     _consumer(consumer)
  {}

  void operator()()
  {
    queue_front<Input> upstream(_input.get());

    Input input;
    while (upstream.wait_pull(input))
    {
      _consumer(std::move(input));
    }

    _promise.set_value();
  }

  queue_back<Input> get_queue_back()
  {
    return queue_back<Input>(_input.get());
  }

private:
  std::promise<void> _promise;
  std::unique_ptr<queue<Input>> _input;
  Consumer _consumer;
};

template <typename Input, typename Consumer>
class multi_consume_output_task
{
public:
  multi_consume_output_task(
    std::promise<void>&& promise,
    const Consumer& consumer
  )
    :_promise(std::move(promise)),
     _input(new queue<Input>()),
     _consumer(consumer)
  {}

  void operator()()
  {
    queue_front<Input> upstream(_input.get());

    while (! upstream.is_empty() || ! upstream.is_closed())
    {
      _consumer(upstream);
    }

    _promise.set_value();
  }

  queue_back<Input> get_queue_back()
  {
    return queue_back<Input>(_input.get());
  }

private:
  std::promise<void> _promise;
  std::unique_ptr<queue<Input>> _input;
  Consumer _consumer;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_TASK_HPP
