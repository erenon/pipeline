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

#ifndef BOOST_PIPELINE_DETAIL_REENTRANT_THREAD_POOL_HPP
#define BOOST_PIPELINE_DETAIL_REENTRANT_THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <functional>

#include <boost/pipeline/detail/sync_queue.hpp>

namespace boost {
namespace pipeline {
namespace detail {

class reentrant_thread_pool
{
public:
  typedef std::function<bool()> work;

  reentrant_thread_pool(const unsigned size = std::thread::hardware_concurrency())
  {
    // if hardware_concurrency == 0:
    const unsigned actual_size = (std::max)(1u, size);

    for (unsigned i = 0; i < actual_size; ++i)
    {
      _threads.emplace_back(&reentrant_thread_pool::worker_thread, this);
    }
  }

  ~reentrant_thread_pool()
  {
    _works.close();

    for (auto& thread : _threads)
    {
      thread.join();
    }
  }

  template <typename Callable>
  void submit(const Callable& w)
  {
    _works.push(work(w));
  }

  template <typename Callable>
  void submit(Callable&& w)
  {
    _works.push(work(std::move(w)));
  }

private:
  typedef sync_queue<work>::op_status op_status;

  void worker_thread()
  {
    auto status = op_status::SUCCESS;

    while (status != op_status::CLOSED)
    {
      work w;
      status = _works.pop(w);

      if (status == op_status::SUCCESS)
      {
        bool finished = w(); // execute work

        if (!finished)
        {
          _works.push(w); // reentrancy
        }
      }
      // else: retry or return if closed
    }
  }

  std::vector<std::thread> _threads;
  sync_queue<work> _works;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_REENTRANT_THREAD_POOL_HPP
