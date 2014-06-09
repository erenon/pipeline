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

#ifndef BOOST_PIPELINE_THREADING_HPP
#define BOOST_PIPELINE_THREADING_HPP

#define BOOST_THREAD_VERSION 4
#define BOOST_THREAD_PROVIDES_EXECUTORS

#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/future.hpp>

namespace boost {
namespace pipeline {

typedef boost::basic_thread_pool thread_pool;

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_THREADING_HPP
