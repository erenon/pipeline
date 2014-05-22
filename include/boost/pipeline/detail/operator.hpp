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

#ifndef BOOST_PIPELINE_OPERATOR_HPP
#define BOOST_PIPELINE_OPERATOR_HPP

#include <type_traits>

#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/range_reader.hpp>
#include <boost/pipeline/detail/connector.hpp>
#include <boost/pipeline/detail/open_segment.hpp>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Connection>
using valid_connection = typename std::enable_if<
  ! std::is_same<Connection, invalid_trafo>::value
,int>::type;


// pipeline::from(input) | trafo
template <
  typename RangeIt, typename Function,
  typename Result = typename connector<range_reader<RangeIt>, Function>::type,
  valid_connection<Result> = 0
>
Result operator|(const range_reader<RangeIt>& range, const Function& f)
{
  return Result(range, f);
}

// TODO the following four with is_segment?

// one_one_segment | trafo
template <
  typename Parent, typename Output, typename Function,
  typename Result = typename connector<one_one_segment<Parent, Output>, Function>::type,
  valid_connection<Result> = 0
>
Result operator|(const one_one_segment<Parent, Output>& segment, const Function& f)
{
  return Result(segment, f);
}

// one_n_segment | trafo
template <
  typename Parent, typename Output,
  typename R, typename Function,
  typename Result = typename connector<one_n_segment<Parent, Output, R>, Function>::type,
  valid_connection<Result> = 0
>
Result operator|(const one_n_segment<Parent, Output, R>& segment, const Function& f)
{
  return Result(segment, f);
}

// n_one_segment | trafo
template <
  typename Parent, typename Output, typename Function,
  typename Result = typename connector<n_one_segment<Parent, Output>, Function>::type,
  valid_connection<Result> = 0
>
Result operator|(const n_one_segment<Parent, Output>& segment, const Function& f)
{
  return Result(segment, f);
}

// n_m_segment | trafo
template <typename Parent, typename Output, typename R, typename Function>
auto operator|(const n_m_segment<Parent, Output, R>& segment, const Function& f)
 -> typename connector<n_m_segment<Parent, Output, R>, Function>::type
{
  typedef typename connector<n_m_segment<Parent, Output, R>, Function>::type result;
  return result(segment, f);
}

template <typename Segment>
using enable_if_segment = typename std::enable_if<
  is_segment<Segment>::value
,int>::type;

// segment | open_segment
template <typename Segment, typename... Trafos, enable_if_segment<Segment> = 0>
auto operator|(const Segment& segment, const open_segment<Trafos...>& open_s)
  -> decltype(open_s.connect_to(segment))
{
  return open_s.connect_to(segment);
}

template <typename RangeReader>
using enable_if_range_reader = typename std::enable_if<
  is_range_reader<RangeReader>::value
,int>::type;

// range_reader | open_segment
template <typename RangeReader, typename... Trafos, enable_if_range_reader<RangeReader> = 0>
auto operator|(const RangeReader& reader, const open_segment<Trafos...>& open_s)
  -> decltype(open_s.connect_to(reader))
{
  return open_s.connect_to(reader);
}

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_OPERATOR_HPP
