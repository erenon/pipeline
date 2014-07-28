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

#include <boost/thread/sync_queue.hpp>

#define BOOST_THREAD_QUEUE_DEPRECATE_OLD

namespace boost {
namespace pipeline {

template <typename T>
using queue = sync_queue<T>;

template <typename T>
class queue_back
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_back(queue<T>* queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  void push(const T& item)
  {
    _queue_ptr->push_back(item);
  }

  void push(T&& item)
  {
    _queue_ptr->push_back(std::forward<T>(item));
  }

  void close()
  {
    _queue_ptr->close();
  }

private:
  queue<T>* _queue_ptr;
};

template <typename T>
class queue_front
{
public:
  typedef typename queue<T>::value_type value_type;

  queue_front(queue<T>* queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  bool wait_pull(T& ret)
  {
    auto status = _queue_ptr->wait_pull_front(ret);
    return (status == queue_op_status::success);
  }

  bool is_empty() const
  {
    return _queue_ptr->empty();
  }

  bool is_closed() const
  {
    return _queue_ptr->closed();
  }

private:
  queue<T>* _queue_ptr;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_QUEUE_HPP
