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

#ifndef BOOST_PIPELINE_DETAIL_CONNECTOR_HPP
#define BOOST_PIPELINE_DETAIL_CONNECTOR_HPP

#include <type_traits>
#include <iterator>

#include <boost/pipeline/detail/segment.hpp>

namespace boost {
namespace pipeline {
namespace detail {

template <typename Plan, typename Callable>
class member_connector;

template <typename Plan, typename Bind>
class bind_connector;

// queue_front predicates
template <typename Input, typename Q>
using if_queue_front = typename std::enable_if<
  std::is_same<typename std::decay<Q>::type, queue_front<Input>>::value
,int>::type;

template <typename Input, typename Q>
using not_queue_front = typename std::enable_if<
  ! std::is_same<typename std::decay<Q>::type, queue_front<Input>>::value
,int>::type;

// if no match found:
struct invalid_trafo {};

/**
 * Connector
 */
template <typename Plan, typename Trafo>
class connector
{
  typedef typename Plan::value_type input;

  template <typename Q>
  using if_queue_front = if_queue_front<input, Q>;

  template <typename Q>
  using not_queue_front = not_queue_front<input, Q>;

  //
  // Connect function pointers
  //

  // Function pointer, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_front<A> = 0>
  static one_one_segment<Plan, R> connect(R(function)(A));

  // Function pointer, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_front<A> = 0>
  static n_one_segment<Plan, R> connect(R(function)(A));

  // Function pointer, two args, first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_front<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R> connect(R(function)(A, B));

  // Function pointer, two args, first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_front<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R> connect(R(function)(A, B));

  //
  // Connect functor types which has operator() but not bind
  // (includes lambda expressions and std::function)
  //

  // Check if Callable has operator()
  template <typename Callable>
  class is_callable
  {
    struct not_callable {};

    template <typename Callable2>
    static decltype(&Callable2::operator()) test(int);

    template <typename Callable2>
    static not_callable test(...);

  public:
    enum { value = ! std::is_same<not_callable, decltype(test<Callable>(0))>::value};
  };

  template <typename Callable, typename std::enable_if<
         is_callable<Callable>::value
    && ! std::is_bind_expression<Callable>::value
  ,int>::type = 0>
  static auto connect(const Callable& lambda)
    -> typename member_connector<Plan, Callable>::type
  ;

  //
  // Connect std::bind expression
  //

  template <typename Bind, typename std::enable_if<
    std::is_bind_expression<Bind>::value
  ,int>::type = 0>
  static auto connect(const Bind& bind)
    -> typename bind_connector<Plan, Bind>::type
  ;

  //
  // Connect container (sink)
  //

  template <typename T, typename Container>
  struct is_container
  {
    struct not_container;

    template <typename Container2>
    static std::back_insert_iterator<Container2> test(int);

    template <typename>
    static not_container test(...);

    struct wrong_type;

    template <typename Container2>
    static typename Container2::value_type testType(int);

    template <typename>
    static wrong_type testType(...);

  public:
    enum { value =
       ! std::is_same<not_container, decltype(test<Container>(0))>::value
    &&   std::is_convertible<T, decltype(testType<Container>(0))>::value
    };
  };

  template <typename Container, typename std::enable_if<
    is_container<typename Plan::value_type, Container>::value
  ,int>::type = 0>
  static range_output_segment<Container, Plan> connect(Container container);

  // If none of the above matches: invalid transformation
  static invalid_trafo connect(...);

public:
  typedef decltype(connect(std::declval<Trafo>())) type;
};

template <typename Plan, typename Callable>
class member_connector
{
  typedef typename Plan::value_type input;

  template <typename Q>
  using if_queue_front = if_queue_front<input, Q>;

  template <typename Q>
  using not_queue_front = not_queue_front<input, Q>;

  // Note: mutable labdas have non-const operator()

  // operator() const, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_front<A> = 0>
  static one_one_segment<Plan, R>
  connect(R(Callable::*function)(A) const);

  // operator() non-const, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_front<A> = 0>
  static one_one_segment<Plan, R>
  connect(R(Callable::*function)(A));

  // operator() const, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_front<A> = 0>
  static n_one_segment<Plan, R>
  connect(R(Callable::*function)(A) const);

  // operator() non-const, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_front<A> = 0>
  static n_one_segment<Plan, R>
  connect(R(Callable::*function)(A));

  // operator() const, two args first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_front<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B) const);

  // operator() non-const, two args first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_front<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B));

  // operator() const, two args first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_front<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B) const);

  // operator() non-const, two args first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_front<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B));

public:
  typedef decltype(connect(&Callable::operator())) type;
};

template <typename Plan, typename Bind>
class bind_connector
{
  struct invalid_bind_trafo {};

  typedef typename Plan::value_type input;
  typedef typename Bind::result_type R;

  // output is R, or try input if R is void
  typedef typename std::conditional<
    std::is_same<void, R>::value, input, R
  >::type output;

  template <typename Bind1, typename R1>
  class is_one_one_trafo
  {
    template <
      typename Bind2,
      typename R2,
      typename std::enable_if<std::is_same<
        R2,
        decltype(std::declval<Bind2>()(
          std::declval<input>()
        ))
      >::value, int>::type = 0
    >
    static int test(int);

    template <typename Bind2, typename R2>
    static long test(...);

  public:
    enum { value = std::is_same<int, decltype(test<Bind1, R1>(0))>::value };
  };

  template <typename Bind1, typename R1>
  class is_n_one_trafo
  {
    template <
      typename Bind2,
      typename R2,
      typename std::enable_if<std::is_same<
        R2,
        decltype(std::declval<Bind2>()(
          std::declval<queue_front<input>&>()
        ))
      >::value, int>::type = 0
    >
    static int test(int);

    template <typename Bind2, typename R2>
    static long test(...);

  public:
    enum { value = std::is_same<int, decltype(test<Bind1, R1>(0))>::value };
  };

  // operator() template, 1-1
  template <typename Bind1, typename std::enable_if<
    is_one_one_trafo<Bind1, output>::value
  ,int>::type = 0>
  static one_one_segment<Plan, output> connectCallOpTemplate(int);

  // operator() template, 1-N
  template <typename Bind1, typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind1>()(
        std::declval<input>(),
        std::declval<boost::pipeline::queue_back<output>&>()
      ))
    >::value
  && ! is_one_one_trafo<Bind1, output>::value
  ,int>::type = 0>
  static one_n_segment<Plan, output, R> connectCallOpTemplate(int);

  // operator() template, N-1
  template <typename Bind1, typename std::enable_if<
    is_n_one_trafo<Bind1, output>::value
  ,int>::type = 0>
  static n_one_segment<Plan, output> connectCallOpTemplate(int);

  // operator() template, N-M
  template <typename Bind1, typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind1>()(
        std::declval<boost::pipeline::queue_front<input>&>(),
        std::declval<boost::pipeline::queue_back<output>&>()
      ))
    >::value
  && ! is_n_one_trafo<Bind1, output>::value
  ,int>::type = 0>
  static n_m_segment<Plan, output, R> connectCallOpTemplate(int);

  template <typename>
  static invalid_bind_trafo connectCallOpTemplate(...);

public:
  typedef decltype(connectCallOpTemplate<Bind>(0)) type;

  static_assert(
    ! std::is_same<type, invalid_bind_trafo>::value,
    "Invalid bind transformation"
  );
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_CONNECTOR_HPP
