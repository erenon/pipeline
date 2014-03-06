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

namespace boost {
namespace pipeline {

namespace {

template <typename T>
constexpr T identity(const T& t)
{
  return t;
}

} // namespace

template <typename Container>
segment<detail::range_reader<typename Container::iterator>, typename Container::value_type>
from(Container& container)
{
  typedef typename Container::value_type value_type;
  typedef detail::range_reader<typename Container::iterator> c_range_reader;

  c_range_reader range(container.begin(), container.end());

  return segment<c_range_reader, value_type>(
    range,
    std::function<value_type(value_type)>(&identity<value_type>)
  );
}

template <typename Iterator>
segment<detail::range_reader<Iterator>, typename Iterator::value_type>
from(const Iterator& begin, const Iterator& end)
{
  typedef typename Iterator::value_type value_type;
  typedef detail::range_reader<Iterator> c_range_reader;

  c_range_reader range(begin, end);

  return segment<c_range_reader, value_type>(
    range,
    std::function<value_type(value_type)>(&identity<value_type>)
  );
}

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_PIPELINE__HPP
