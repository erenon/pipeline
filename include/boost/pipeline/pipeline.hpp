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

#ifndef BOOST_PIPELINE_PIPELINE__HPP
#define BOOST_PIPELINE_PIPELINE__HPP

#include <boost/pipeline/segment.hpp>
#include <boost/pipeline/detail/range_reader.hpp>
#include <boost/pipeline/detail/operator.hpp>

namespace boost {
namespace pipeline {

/**
 * Creates a range_reader operating on `container`.
 */
template <typename Container>
detail::range_reader<typename Container::iterator>
from(Container& container)
{
  typedef detail::range_reader<typename Container::iterator> c_range_reader;

  return c_range_reader(container.begin(), container.end());
}

/**
 * Creates a range_reader operating on a range.
 */
template <typename Iterator>
detail::range_reader<Iterator>
from(const Iterator& begin, const Iterator& end)
{
  typedef detail::range_reader<Iterator> c_range_reader;

  return c_range_reader(begin, end);
}

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_PIPELINE__HPP
