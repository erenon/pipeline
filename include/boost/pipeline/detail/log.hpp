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

//
// This file is TEMPORARY and should be removed later.
//

#ifndef BOOST_PIPELINE_DETAIL_LOG_HPP
#define BOOST_PIPELINE_DETAIL_LOG_HPP

#include <cstdio>

namespace boost {
namespace pipeline {

#define LOG0(msg) printf(msg "\n"); fflush(stdout)
#define LOG(msg, ...) printf(msg "\n", ##__VA_ARGS__); fflush(stdout)

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_LOG_HPP
