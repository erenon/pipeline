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

/**
 * Producer handle to buffer queue between segments.
 *
 * Each transformation producing multiple items on each call
 * receive a `queue_back` instance which is used to access to
 * the downstream queue.
 *
 * A handle is valid until the downstream segment terminates.
 *
 * **Template arguments**:
 *
 * - @b T Value type of the underlying queue
 */
template <typename T>
class queue_back
{
public:
  /** Value type of the underlying queue */
  typedef typename queue<T>::value_type value_type;

  /**
   * Creates a handle to the given queue.
   *
   * The constructed object does *not* take ownership
   * of the give queue.
   *
   * @param queue_ptr Queue to be accessed
   */
  queue_back(queue<T>* queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  /**
   * Pushes an item to the underlying queue.
   *
   * @param item Item to be added to the queue
   * @pre queue is not closed
   * @throws `boost::sync_queue_is_closed` If the queue is already closed
   */
  void push(const T& item)
  {
    _queue_ptr->push_back(item);
  }

  /** @copydoc push */
  void push(T&& item)
  {
    _queue_ptr->push_back(std::forward<T>(item));
  }

  /**
   * Closes the underlying queue.
   *
   * Queue will not receive additional items.
   */
  void close()
  {
    _queue_ptr->close();
  }

private:
  queue<T>* _queue_ptr;
};

/**
 * Consumer handle to a buffer queue between segments.
 *
 * Each transformation consuming multiple items on each call
 * receive a `queue_front` instance which is used to access to
 * the upstream queue.
 *
 * A handle is valid until the segment which owns the input
 * queue terminates.
 *
 * **Template arguments**:
 *
 * - @b T Value type of the underlying queue
 */
template <typename T>
class queue_front
{
public:
  /** Value type of the underlying queue */
  typedef typename queue<T>::value_type value_type;

  /**
   * Creates a handle to the given queue.
   *
   * The constructed object does *not* take ownership
   * of the give queue.
   *
   * @param queue_ptr Queue to be accessed
   */
  queue_front(queue<T>* queue_ptr)
    :_queue_ptr(queue_ptr)
  {}

  /**
   * Pulls the front item of the queue.
   *
   * Blocks until an item becomes available
   * or the underlying queue gets closed.
   *
   * @param ret Pulled item, if any
   * @returns true, if pulled successfully, false otherwise (queue is closed)
   */
  bool wait_pull(T& ret)
  {
    auto status = _queue_ptr->wait_pull_front(ret);
    return (status == queue_op_status::success);
  }

  /**
   * Checks the underlying queue for emptiness.
   *
   * This call is subject to race.
   *
   * @returns true, if the queue was empty, false otherwise
   */
  bool is_empty() const
  {
    return _queue_ptr->empty();
  }

  /**
   * Checks whether the underlying queue is closed.
   *
   * @returns true, if the queue is closed, false otherwise
   */
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
