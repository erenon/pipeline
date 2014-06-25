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

template <typename T>
class queue
{
  typedef boost::lockfree::spsc_queue<T, boost::lockfree::capacity<1 << 10>> buffer;

public:
  enum class op_status { SUCCESS, FAILURE, CLOSED };

  typedef T value_type;

  queue() = default;
  queue(const queue&) = delete;
  queue& operator=(const queue&) = delete;

  op_status try_push(const T& item)
  {
    auto success = _buffer.push(item);
    if (!success)
    {
      return op_status::FAILURE; // buffer is full
    }

    return op_status::SUCCESS;
  }

  op_status try_pop(T& ret)
  {
    auto success = _buffer.pop(ret);
    if (!success)
    {
      return (_closed) ? op_status::CLOSED : op_status::FAILURE;
                         // no more items    // buffer is empty
    }

    return op_status::SUCCESS;
  }

  op_status try_pop()
  {
    auto success = _buffer.pop();
    if (!success)
    {
      return (_closed) ? op_status::CLOSED : op_status::FAILURE;
                         // no more items    // buffer is empty
    }

    return op_status::SUCCESS;
  }

  bool empty() const
  {
    return _buffer.read_available() == 0;
  }

  bool full() const
  {
    return _buffer.write_available() == 0;
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
class queue_front
{
public:
  typedef typename queue<T>::op_status op_status;
  typedef typename queue<T>::value_type value_type;

  queue_front(const queuePtr<T>& queuePtr)
    :_queuePtr(queuePtr)
  {}

  op_status try_push(const T& item)
  {
    return _queuePtr->try_push(item);
  }
  bool full() const
  {
    return _queuePtr->full();
  }

  void close() { _queuePtr->close(); }

private:
  queuePtr<T> _queuePtr;
};

template <typename T>
class queue_back
{
public:
  typedef typename queue<T>::op_status op_status;
  typedef typename queue<T>::value_type value_type;

  queue_back(const queuePtr<T>& queuePtr)
    :_queuePtr(queuePtr)
  {}

  op_status try_pop(T& ret)
  {
    return _queuePtr->try_pop(ret);
  }

  op_status try_pop()
  {
    return _queuePtr->try_pop();
  }

  bool empty() const
  {
    return _queuePtr->empty();
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
