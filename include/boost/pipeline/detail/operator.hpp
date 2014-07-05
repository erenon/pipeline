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

#ifndef BOOST_PIPELINE_DETAIL_OPERATOR_HPP
#define BOOST_PIPELINE_DETAIL_OPERATOR_HPP

#include <type_traits>

#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/connector.hpp>
#include <boost/pipeline/detail/open_segment.hpp>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Connection>
using valid_connection = typename std::enable_if<
  ! std::is_same<Connection, invalid_trafo>::value
,int>::type;


template <typename Segment>
using enable_if_segment = typename std::enable_if<
  is_connectable_segment<Segment>::value
,int>::type;

// segment | transformation
template <
  typename Segment, typename Function,
  enable_if_segment<Segment> = 0,
  typename Result = typename connector<Segment, Function>::type,
  valid_connection<Result> = 0
>
Result operator|(const Segment& segment, const Function& f)
{
  return Result(segment, f);
}

// segment | container
template <
  typename Segment, typename Container,
  enable_if_segment<Segment> = 0,
  typename Result = typename connector<Segment, Container>::type,
  valid_connection<Result> = 0
>
Result operator|(const Segment& segment, Container& c)
{
  return Result(segment, c);
}

// segment | open_segment
template <typename Segment, typename... Trafos, enable_if_segment<Segment> = 0>
auto operator|(const Segment& segment, const open_segment<Trafos...>& open_s)
  -> decltype(open_s.connect_to(segment))
{
  return open_s.connect_to(segment);
}

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_OPERATOR_HPP
