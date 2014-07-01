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
using queuePtr = std::shared_ptr<queue<T>>;

template <typename T>
class queue_back
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_back(const queuePtr<T>& queuePtr)
    :_queuePtr(queuePtr)
  {}

  queue_op_status try_push(const T& item)
  {
    return _queuePtr->try_push(item);
  }

  bool is_full() const
  {
    return _queuePtr->is_full();
  }

  size_t write_available() const
  {
    return _queuePtr->write_available();
  }

  void close() { _queuePtr->close(); }

private:
  queuePtr<T> _queuePtr;
};

template <typename T>
class queue_front
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_front(const queuePtr<T>& queuePtr)
    :_queuePtr(queuePtr)
  {}

  queue_op_status try_pop(T& ret)
  {
    return _queuePtr->try_pop(ret);
  }

  queue_op_status try_pop()
  {
    return _queuePtr->try_pop();
  }

  bool is_empty() const
  {
    return _queuePtr->is_empty();
  }

  size_t read_available() const
  {
    return _queuePtr->read_available();
  }

  const T& front() const
  {
    return _queuePtr->front();
  }

  bool is_closed() const { return _queuePtr->is_closed(); }

private:
  queuePtr<T> _queuePtr;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_QUEUE_HPP
