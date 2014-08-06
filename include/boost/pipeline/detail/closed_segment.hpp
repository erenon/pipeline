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

#ifndef BOOST_PIPELINE_DETAIL_CLOSED_SEGMENT_HPP
#define BOOST_PIPELINE_DETAIL_CLOSED_SEGMENT_HPP

namespace boost {
namespace pipeline {
namespace detail {

template <typename Transformation>
struct closed_segment
{
  Transformation transformation;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_CLOSED_SEGMENT_HPP
