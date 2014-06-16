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

#ifndef BOOST_PIPELINE_DETAIL_SYNC_QUEUE_HPP
#define BOOST_PIPELINE_DETAIL_SYNC_QUEUE_HPP

#include <deque>
#include <mutex>
#include <condition_variable>

namespace boost {
namespace pipeline {
namespace detail {

template <typename T>
class sync_queue
{
public:
  enum class op_status { SUCCESS, FAILURE, CLOSED };

  typedef T value_type;

  sync_queue() = default;
  sync_queue(const sync_queue&) = delete;
  sync_queue& operator=(const sync_queue&) = delete;

  void push(const T& item)
  {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _buffer.push_back(item);
    }

    _signal.notify_one();
  }

  void push(T&& item)
  {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _buffer.push_back(std::move(item));
    }

    _signal.notify_one();
  }

  op_status pop(T& item)
  {
    std::unique_lock<std::mutex> lock(_mutex);

    while (_buffer.empty() && !_closed)
    {
      _signal.wait(lock);
    }

    if ( ! _buffer.empty())
    {
      item = _buffer.front();
      _buffer.pop_front();
      return op_status::SUCCESS;
    }
    else if (_closed)
    {
      return op_status::CLOSED;
    }
    else
    {
      return op_status::FAILURE;
    }
  }

  bool is_closed() const { return _closed; }

  void close()
  {
    _closed = true;
    _signal.notify_all();
  }

private:
  std::deque<T> _buffer;
  std::mutex _mutex;
  std::condition_variable _signal;
  bool _closed = false;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_SYNC_QUEUE_HPP
