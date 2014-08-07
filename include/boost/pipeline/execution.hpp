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

#ifndef BOOST_PIPELINE_EXECUTION_HPP
#define BOOST_PIPELINE_EXECUTION_HPP

#include <future>
#include <chrono>

#include <boost/pipeline/queue.hpp>
#include <boost/pipeline/threading.hpp>

namespace boost {
namespace pipeline {

/**
 * Handle to an executing pipeline.
 *
 * An instance of this class returned every time
 * `run()` is called on a complete pipeline.
 *
 * Holds a future which is set on pipeline completition,
 * destructor blocks until pipeline is terminated.
 * `std::move` it if it's not desired.
 */
class execution
{
public:
  /**
   * Creates an exectution holding `future`
   *
   * @param future Future promised by the terminating segment
   * of the represented pipeline.
   */
  execution(std::future<void>&& future)
    :_future(std::move(future))
  {}

  /**
   * Checks if the pipeline has terminated
   *
   * @returns true, if the execution of the pipeline is done, false otherwise
   */
  bool is_done()
  {
    return _future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready;
  }

  /**
   * Waits until the execution of the pipeline is completed
   *
   * @post Blocks until the execution is done
   */
  void wait()    { _future.wait(); }

private:
  std::future<void> _future;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_EXECUTION_HPP
