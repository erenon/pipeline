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

#ifndef BOOST_PIPELINE_TYPE_ERASURE_HPP
#define BOOST_PIPELINE_TYPE_ERASURE_HPP

#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/member.hpp>

#include <boost/pipeline/threading.hpp>
#include <boost/pipeline/execution.hpp>
#include <boost/pipeline/queue.hpp>

BOOST_TYPE_ERASURE_MEMBER((boost)(pipeline)(has_run), run)

namespace boost {
namespace pipeline {

typedef void terminated;

template<typename Value, typename T = typename type_erasure::_self>
struct value_type_is_same_as
{
  static void apply(const T&)
  {
    static_assert(
      std::is_same<typename T::value_type, Value>::value,
      "Invalid segment type, value_type mismatch"
    );
  }
};

template<typename Root, typename T = typename type_erasure::_self>
struct root_type_is_same_as
{
  static void apply(const T&)
  {
    static_assert(
      std::is_same<typename T::root_type, Root>::value,
      "Invalid segment type, root_type mismatch"
    );
  }
};

template <typename Input, typename Output>
using segment = typename type_erasure::any<
  mpl::vector<
    type_erasure::copy_constructible<>,
    root_type_is_same_as<Input>,
    value_type_is_same_as<Output>,
    typename std::conditional<
      std::is_same<Output, terminated>::value,
      has_run<execution(thread_pool&)>,
      has_run<queue_front<Output>(thread_pool&)>
    >::type,
    type_erasure::relaxed
  >
>;

//
// is_connectable_segment predicate specializations
//

namespace detail {

template <typename>
struct is_connectable_segment;

template <typename T>
struct is_connectable_segment<segment<terminated, T>> : public std::true_type {};

} // namespace detail

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_TYPE_ERASURE_HPP
