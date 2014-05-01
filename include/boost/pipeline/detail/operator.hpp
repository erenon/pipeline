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

#include <boost/pipeline/segment.hpp>
#include <boost/pipeline/detail/range_reader.hpp>
#include <boost/pipeline/detail/connector.hpp>

namespace boost {
namespace pipeline {
namespace detail {

// pipeline::from(input) | trafo
template <typename RangeIt, typename Function>
auto operator|(const range_reader<RangeIt>& range, const Function& f)
 -> typename connector<range_reader<RangeIt>, Function>::type
{
  typedef typename connector<range_reader<RangeIt>, Function>::type result;
  return result(range, f);
}

// one_one_segment | trafo
template <typename Parent, typename Output, typename Function>
auto operator|(const one_one_segment<Parent, Output>& segment, const Function& f)
 -> typename connector<one_one_segment<Parent, Output>, Function>::type
{
  typedef typename connector<one_one_segment<Parent, Output>, Function>::type result;
  return result(segment, f);
}

// one_n_segment | trafo
template <typename Parent, typename Output, typename R, typename Function>
auto operator|(const one_n_segment<Parent, Output, R>& segment, const Function& f)
 -> typename connector<one_n_segment<Parent, Output, R>, Function>::type
{
  typedef typename connector<one_n_segment<Parent, Output, R>, Function>::type result;
  return result(segment, f);
}

// n_one_segment | trafo
template <typename Parent, typename Output, typename Function>
auto operator|(const n_one_segment<Parent, Output>& segment, const Function& f)
 -> typename connector<n_one_segment<Parent, Output>, Function>::type
{
  typedef typename connector<n_one_segment<Parent, Output>, Function>::type result;
  return result(segment, f);
}

// n_m_segment | trafo
template <typename Parent, typename Output, typename R, typename Function>
auto operator|(const n_m_segment<Parent, Output, R>& segment, const Function& f)
 -> typename connector<n_m_segment<Parent, Output, R>, Function>::type
{
  typedef typename connector<n_m_segment<Parent, Output, R>, Function>::type result;
  return result(segment, f);
}

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_OPERATOR_HPP
