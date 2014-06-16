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

#include <future>

#include <boost/pipeline/detail/reentrant_thread_pool.hpp>

namespace boost {
namespace pipeline {

typedef detail::reentrant_thread_pool thread_pool;

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_THREADING_HPP
