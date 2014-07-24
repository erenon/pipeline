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
#include <boost/type_erasure/call.hpp>
#include <boost/type_erasure/binding.hpp>
#include <boost/type_erasure/rebind_any.hpp>

#include <boost/pipeline/threading.hpp>
#include <boost/pipeline/execution.hpp>
#include <boost/pipeline/queue.hpp>
#include <boost/pipeline/detail/segment.hpp>

BOOST_TYPE_ERASURE_MEMBER((boost)(pipeline)(detail)(has_run), run)

namespace boost {

namespace pipeline {

namespace detail {

struct unknown_type {};

template <typename Root, typename T = typename type_erasure::_self>
struct root_type_is_same_as
{
  static void apply(const T&)
  {
    static_assert(
       std::is_same<typename T::root_type, Root>::value
    || std::is_same<typename T::root_type, detail::unknown_type>::value,
      "Invalid segment type, root_type mismatch"
    );
  }
};

template <typename Value, typename T = typename type_erasure::_self>
struct value_type_is_same_as
{
  static void apply(const T&)
  {
    static_assert(
       std::is_same<typename T::value_type, Value>::value
    || std::is_same<typename T::value_type, detail::unknown_type>::value,
      "Invalid segment type, value_type mismatch"
    );
  }
};

template <typename Segment, typename Parent>
struct has_connect_to
{
  static auto apply(const Segment& segment, const Parent& parent)
    -> decltype(segment.connect_to(std::declval<Parent>()))
  {
    return segment.connect_to(parent);
  }
};

} // namespace detail

typedef void terminated;

template <typename Input, typename Output>
using segment_concept = typename mpl::vector<
  type_erasure::copy_constructible<>,

  detail::root_type_is_same_as<Input>,   // or unknown
  detail::value_type_is_same_as<Output>, // or unknown

  // if left terminated
  //   if right terminated: has_run<execution>
  //   else has_run<queue_front>
  // else open_segment
  typename std::conditional<
    std::is_same<Input, terminated>::value,
    typename std::conditional<
      std::is_same<Output, terminated>::value,
      detail::has_run<execution(thread_pool&)>,
      detail::has_run<queue_front<Output>(thread_pool&)>
    >::type,
    detail::has_connect_to<type_erasure::_self, type_erasure::_a>
  >::type,

  type_erasure::relaxed
>;

template <typename Input, typename Output>
using segment = typename type_erasure::any<segment_concept<Input, Output>>;

namespace detail {

//
// is_connectable_segment predicate specializations
//

template <typename>
struct is_connectable_segment;

template <typename T>
struct is_connectable_segment<segment<terminated, T>> : public std::true_type {};

} // namespace detail

} // namespace pipeline

namespace type_erasure {

template <typename Concept, typename Value, typename Base>
struct concept_interface<::boost::pipeline::detail::root_type_is_same_as<Value>, Base, Concept> : Base
{
  typedef Value root_type;
};

template <typename Concept, typename Value, typename Base>
struct concept_interface<::boost::pipeline::detail::value_type_is_same_as<Value>, Base, Concept> : Base
{
  typedef Value value_type;
};

template <typename Segment, typename Parent, typename Base>
struct concept_interface<pipeline::detail::has_connect_to<Segment, Parent>, Base, Segment>
  : public Base
{
  typedef mpl::map<
    mpl::pair<_self, Segment>,
    mpl::pair<_a, Parent>
  > binding_map;

  typedef binding<typename concept_of<Base>::type> segment_binding;

  auto connect_to(typename rebind_any<Base, _a>::type parent)
    -> decltype(
           call(segment_binding{binding_map{}}, pipeline::detail::has_connect_to<Segment, Parent>(), *this, parent)
       )
  {
    return call(segment_binding{binding_map{}}, pipeline::detail::has_connect_to<Segment, Parent>(), *this, parent);
  }
};

} // namespace type_erasure

} // namespace boost

#endif // BOOST_PIPELINE_TYPE_ERASURE_HPP
