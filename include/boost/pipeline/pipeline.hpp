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

#include <type_traits>

#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/range_reader.hpp>
#include <boost/pipeline/detail/operator.hpp>
#include <boost/pipeline/detail/open_segment.hpp>

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

/**
 * Creates on open_segment representing `function`
 */
template <typename Function, typename std::enable_if<
  ! std::is_function<Function>::value
,int>::type = 0>
detail::open_segment<Function>
make(const Function& function)
{
  return detail::open_segment<Function>(function);
}

/**
 * Creates on open_segment representing `function`,
 * when `function` is of a function type.
 */
template <typename Function, typename std::enable_if<
  std::is_function<Function>::value
,int>::type = 0>
detail::open_segment<typename std::add_pointer<Function>::type>
make(const Function& function)
{
  return detail::open_segment<typename std::add_pointer<Function>::type>(&function);
}

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_PIPELINE__HPP
