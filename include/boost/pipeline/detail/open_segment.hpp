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

#ifndef BOOST_PIPELINE_OPEN_SEGMENT_HPP
#define BOOST_PIPELINE_OPEN_SEGMENT_HPP

#include <tuple>
#include <type_traits>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Tuple, std::size_t N>
struct foreach_tuple_element
{
  template <typename Segment>
  static auto connect_to(const Segment& segment, const Tuple& tuple)
    -> decltype(
      foreach_tuple_element<Tuple, N-1>::connect_to(segment, tuple) | std::get<N>(tuple)
    )
  {
    auto next_segment =
      foreach_tuple_element<Tuple, N-1>::connect_to(segment, tuple);
    return next_segment | std::get<N>(tuple);
  }
};

template <typename Tuple>
struct foreach_tuple_element<Tuple, 0>
{
  template <typename Segment>
  static auto connect_to(const Segment& segment, const Tuple& tuple)
    -> decltype(segment | std::get<0>(tuple))
  {
    return segment | std::get<0>(tuple);
  }
};

template <typename... Trafos>
class open_segment
{
public:
  typedef std::tuple<Trafos...> Tuple;

private:
  template <typename... Trafos2>
  friend class open_segment;

  /** Constructor for connecting two open plans */
  template <typename Tuple1, typename Tuple2>
  open_segment(const Tuple1& tuple1, const Tuple2& tuple2)
    :_trafos(std::tuple_cat(tuple1, tuple2))
  {}

public:
  /** Create plan from a single transformation */
  template <typename Trafo>
  open_segment(const Trafo trafo)
    :_trafos(trafo)
  {}

  /** Connect two open plans */
  template <typename... Trafos2>
  open_segment<Trafos..., Trafos2...>
  operator|(const open_segment<Trafos2...>& rhs) const
  {
    return open_segment<Trafos..., Trafos2...>(_trafos, rhs._trafos);
  }

  /** Append non-function pointer transformation */
  template <typename Trafo, typename std::enable_if<
    ! std::is_function<Trafo>::value
  ,int>::type = 0>
  open_segment<Trafos..., Trafo>
  operator|(const Trafo& trafo) const
  {
    return open_segment<Trafos..., Trafo>(_trafos, std::make_tuple(trafo));
  }

  /** Append function pointer transformation */
  template <typename Trafo, typename std::enable_if<
    std::is_function<Trafo>::value
  ,int>::type = 0>
  open_segment<Trafos..., typename std::add_pointer<Trafo>::type>
  operator|(const Trafo& trafo) const
  {
    return open_segment<Trafos..., typename std::add_pointer<Trafo>::type>(
      _trafos, std::make_tuple(&trafo)
    );
  }

  /** Connect this to `segment` */
  template <typename Segment>
  auto connect_to(const Segment& segment) const
    -> decltype(
      foreach_tuple_element<Tuple, std::tuple_size<Tuple>::value - 1>
        ::connect_to(segment, std::declval<Tuple>())
    )
  {
    return foreach_tuple_element<Tuple, std::tuple_size<Tuple>::value - 1>
      ::connect_to(segment, _trafos);
  }

private:
  std::tuple<Trafos...> _trafos; /**< Open ended transformations */
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_OPEN_SEGMENT_HPP
