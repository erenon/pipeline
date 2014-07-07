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

#ifndef BOOST_PIPELINE_QUEUE_HPP
#define BOOST_PIPELINE_QUEUE_HPP

#include <memory>

#include <boost/lockfree/spsc_queue.hpp>

namespace boost {
namespace pipeline {

enum class queue_op_status { SUCCESS, FULL, EMPTY, CLOSED };

template <typename T>
class queue
{
  typedef boost::lockfree::spsc_queue<T, boost::lockfree::capacity<1 << 10>> buffer;

public:
  typedef T value_type;

  queue() = default;
  queue(const queue&) = delete;
  queue& operator=(const queue&) = delete;

  queue_op_status try_push(const T& item)
  {
    auto success = _buffer.push(item);
    if (!success)
    {
      return queue_op_status::FULL;
    }

    return queue_op_status::SUCCESS;
  }

  queue_op_status try_pop(T& ret)
  {
    auto success = _buffer.pop(ret);
    if (!success)
    {
      return (_closed) ? queue_op_status::CLOSED : queue_op_status::EMPTY;
    }

    return queue_op_status::SUCCESS;
  }

  queue_op_status try_pop()
  {
    auto success = _buffer.pop();
    if (!success)
    {
      return (_closed) ? queue_op_status::CLOSED : queue_op_status::EMPTY;
    }

    return queue_op_status::SUCCESS;
  }

  bool is_empty() const
  {
    return _buffer.read_available() == 0;
  }

  bool is_full() const
  {
    return _buffer.write_available() == 0;
  }

  size_t read_available() const
  {
    return _buffer.read_available();
  }

  size_t write_available() const
  {
    return _buffer.write_available();
  }

  const T& front() const
  {
    return _buffer.front();
  }

  bool is_closed() const { return _closed; }
  void close() { _closed = true; }

private:
  bool _closed = false;
  buffer _buffer;
};

template <typename T>
using queue_ptr = std::shared_ptr<queue<T>>;

template <typename T>
class queue_back
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_back(const queue_ptr<T>& queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  queue_op_status try_push(const T& item)
  {
    return _queue_ptr->try_push(item);
  }

  bool is_full() const
  {
    return _queue_ptr->is_full();
  }

  size_t write_available() const
  {
    return _queue_ptr->write_available();
  }

  void close() { _queue_ptr->close(); }

private:
  queue_ptr<T> _queue_ptr;
};

template <typename T>
class queue_front
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_front(const queue_ptr<T>& queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  queue_op_status try_pop(T& ret)
  {
    return _queue_ptr->try_pop(ret);
  }

  queue_op_status try_pop()
  {
    return _queue_ptr->try_pop();
  }

  bool is_empty() const
  {
    return _queue_ptr->is_empty();
  }

  size_t read_available() const
  {
    return _queue_ptr->read_available();
  }

  const T& front() const
  {
    return _queue_ptr->front();
  }

  bool is_closed() const { return _queue_ptr->is_closed(); }

private:
  queue_ptr<T> _queue_ptr;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_QUEUE_HPP
