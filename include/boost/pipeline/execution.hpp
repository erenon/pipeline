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

class execution
{
public:
  execution(std::future<bool>&& future)
    :_future(std::move(future))
  {}

  bool is_done()
  {
    return _future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready;
  }

  void wait()    { _future.wait(); }

private:
  std::future<bool> _future;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_EXECUTION_HPP
