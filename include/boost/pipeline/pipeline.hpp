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

#ifndef BOOST_PIPELINE_PIPELINE_HPP
#define BOOST_PIPELINE_PIPELINE_HPP

#include <type_traits>

#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/operator.hpp>
#include <boost/pipeline/detail/open_segment.hpp>

namespace boost {
namespace pipeline {

/**
 * Creates an input_segment operating on `container`.
 */
template <typename Container>
detail::input_segment<typename Container::iterator>
from(Container& container)
{
  typedef detail::input_segment<typename Container::iterator> c_input_segment;

  return c_input_segment(container.begin(), container.end());
}

/**
 * Creates an input_segment operating on a range.
 */
template <typename Iterator>
detail::input_segment<Iterator>
from(const Iterator& begin, const Iterator& end)
{
  typedef detail::input_segment<Iterator> range_input_segment;

  return range_input_segment(begin, end);
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

#endif // BOOST_PIPELINE_PIPELINE_HPP
