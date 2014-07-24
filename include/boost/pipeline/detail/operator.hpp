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

#include <boost/pipeline/type_erasure.hpp>
#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/connector.hpp>
#include <boost/pipeline/detail/open_segment.hpp>
#include <boost/pipeline/detail/closed_segment.hpp>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Connection>
using valid_connection = typename std::enable_if<
  ! std::is_same<Connection, invalid_trafo>::value
,int>::type;


template <typename Segment>
using enable_if_connectable = typename std::enable_if<
  is_connectable_segment<Segment>::value
,int>::type;

// segment | transformation
template <
  typename Segment, typename Function,
  enable_if_connectable<Segment> = 0,
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
  enable_if_connectable<Segment> = 0,
  typename Result = typename connector<Segment, Container>::type,
  valid_connection<Result> = 0
>
Result operator|(const Segment& segment, Container& c)
{
  return Result(segment, c);
}

// segment | open_segment
template <typename Segment, typename... Trafos, enable_if_connectable<Segment> = 0>
auto operator|(const Segment& segment, const open_segment<Trafos...>& open_s)
  -> decltype(open_s.connect_to(segment))
{
  return open_s.connect_to(segment);
}

// segment | closed_segment
template <
  typename Segment, typename Trafo,
  enable_if_connectable<Segment> = 0,
  typename OpenResult = typename connector<Segment, Trafo>::type,
  typename Result = typename to_sink_segment<OpenResult>::type
>
Result operator|(const Segment& segment, const closed_segment<Trafo>& closed)
{
  return Result(segment, closed.transformation);
}

// queue | transformation / segment / open_segment / closed_segment
template <typename T, typename Connectable>
auto operator|(queue<T>& queue, const Connectable& connectable)
  -> decltype(queue_input_segment<T>(queue) | connectable)
{
  return queue_input_segment<T>(queue) | connectable;
}

// segment | segment
template <
  typename LeftIn, typename Middle, typename RightOut,
  typename std::enable_if<
    ! std::is_same<Middle, terminated>::value
  ,int>::type = 0
>
segment<LeftIn, RightOut>
operator|(const segment<LeftIn, Middle>& lhs, const segment<Middle, RightOut>& rhs)
{
  return rhs.connect_to(lhs);
}

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_OPERATOR_HPP
