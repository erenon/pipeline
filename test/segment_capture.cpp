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

#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <functional>

#include <cxxabi.h>

#include <boost/pipeline.hpp>
#include <boost/preprocessor/stringize.hpp>

#define BOOST_TEST_MODULE SegmentCapture
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

struct invalid_transformation;

template <typename Plan, typename Callable>
class member_connector;

template <typename Plan, typename Bind>
class bind_connector;

template <typename Plan, typename Trafo>
class connector
{
  typedef typename Plan::value_type input;

  template <typename Q>
  using if_queue_back = typename std::enable_if<
    std::is_same<typename std::decay<Q>::type, queue_back<input>>::value
  ,int>::type;

  template <typename Q>
  using not_queue_back = typename std::enable_if<
    ! std::is_same<typename std::decay<Q>::type, queue_back<input>>::value
  ,int>::type;

  //
  // Connect function pointers
  //

  // Function pointer, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_back<A> = 0>
  static one_one_segment<Plan, R> connect(R(function)(A));

  // Function pointer, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_back<A> = 0>
  static n_one_segment<Plan, R> connect(R(function)(A));

  // Function pointer, two args, first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_back<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R> connect(R(function)(A, B));

  // Function pointer, two args, first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_back<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R> connect(R(function)(A, B));

  //
  // Connect std::function
  //

//  // std::function, single non-queue arg -> 1-1 Trafo
//  template <typename R, typename A, not_queue_back<A> = 0>
//  static one_one_segment<Plan, R> connect(const std::function<R(A)>& function);
//
//  // std::function, single queue arg -> N-1 Trafo (Aggregation)
//  template <typename R, typename A, if_queue_back<A> = 0>
//  static n_one_segment<Plan, R> connect(const std::function<R(A)>& function);
//
//  // std::function, two args, first is non queue -> 1-N Trafo
//  template <typename R, typename A, typename B, not_queue_back<A> = 0>
//  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
//  connect(const std::function<R(A, B)>& function);
//
//  // std::function, two args, first queue -> N-M Trafo
//  template <typename R, typename A, typename B, if_queue_back<A> = 0>
//  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
//  connect(const std::function<R(A, B)>& function);


  // Connect functor type (includes lambda expressions and std::function)
  // which has operator()
  template <typename Callable, typename std::enable_if<
         std::is_class<Callable>::value
    && ! std::is_bind_expression<Callable>::value
  ,int>::type = 0>
  static auto connect(const Callable& lambda)
    -> typename member_connector<Plan, Callable>::type
  ;

  // Connect std::bind expression
  template <typename Bind, typename std::enable_if<
    std::is_bind_expression<Bind>::value
  ,int>::type = 0>
  static auto connect(const Bind& bind)
    -> typename bind_connector<Plan, Bind>::type
  ;

//  static invalid_transformation connect(...);
public:
  typedef decltype(connect(std::declval<Trafo>())) type;
};

template <typename Plan, typename Callable>
class member_connector
{
  typedef typename Plan::value_type input;

  template <typename Q>
  using if_queue_back = typename std::enable_if<
    std::is_same<typename std::decay<Q>::type, queue_back<input>>::value
  ,int>::type;

  template <typename Q>
  using not_queue_back = typename std::enable_if<
    ! std::is_same<typename std::decay<Q>::type, queue_back<input>>::value
  ,int>::type;

  // Note: mutable labdas have non-const operator()

  // operator() const, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_back<A> = 0>
  static one_one_segment<Plan, R> connect(R(Callable::*function)(A) const);

  // operator() non-const, single non-queue arg -> 1-1 Trafo
  template <typename R, typename A, not_queue_back<A> = 0>
  static one_one_segment<Plan, R> connect(R(Callable::*function)(A));

  // operator() const, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_back<A> = 0>
  static n_one_segment<Plan, R> connect(R(Callable::*function)(A) const);

  // operator() non-const, single queue arg -> N-1 Trafo (Aggregation)
  template <typename R, typename A, if_queue_back<A> = 0>
  static n_one_segment<Plan, R> connect(R(Callable::*function)(A));

  // operator() const, two args first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_back<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B) const);

  // operator() const, two args first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_back<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A, B) const);

  // operator() non-const, two args first is non queue -> 1-N Trafo
  template <typename R, typename A, typename B, not_queue_back<A> = 0>
  static one_n_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A));

  // operator() non-const, two args first queue -> N-M Trafo
  template <typename R, typename A, typename B, if_queue_back<A> = 0>
  static n_m_segment<Plan, typename std::remove_reference<B>::type::value_type, R>
  connect(R(Callable::*function)(A));

public:
  typedef decltype(connect(&Callable::operator())) type;
};

template <typename Plan, typename Bind>
class bind_connector
{
  typedef typename Plan::value_type input;
  typedef typename Bind::result_type R;

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
          std::declval<queue_back<input>&>()
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
    is_one_one_trafo<Bind1, R>::value
  ,int>::type = 0>
  static one_one_segment<Plan, R> connectCallOpTemplate();

  // operator() template, 1-N
  template <typename Bind1, typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind1>()(
        std::declval<input>(),
        std::declval<boost::pipeline::queue_front<R>&>()
      ))
    >::value
  && ! is_one_one_trafo<Bind1, R>::value
  ,int>::type = 0>
  static one_n_segment<Plan, R, R> connectCallOpTemplate();

  // operator() template, N-1
  template <typename Bind1, typename std::enable_if<
    is_n_one_trafo<Bind1, R>::value
  ,int>::type = 0>
  static n_one_segment<Plan, R> connectCallOpTemplate();

  // operator() template, N-M
  template <typename Bind1, typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind1>()(
        std::declval<boost::pipeline::queue_back<input>&>(),
        std::declval<boost::pipeline::queue_front<R>&>()
      ))
    >::value
  && ! is_n_one_trafo<Bind1, R>::value
  ,int>::type = 0>
  static n_m_segment<Plan, R, R> connectCallOpTemplate();

public:
  typedef decltype(connectCallOpTemplate<Bind>()) type;
};

std::size_t len1(const std::string& str)
{
  return str.size();
}

void len2(const std::string& str, queue_front<std::size_t>& out)
{
  auto len = str.size();
  if (len >= 3)
  {
    out.push_back(len);
  }
}

void len3(
  const queue_back<std::string>& in,
  queue_front<std::size_t>& out
)
{
  for (const auto& str : in)
  {
    out.push_back(str.size());
  }
}

struct dummy_plan
{
  typedef std::string value_type;
};

class capture
{
  static std::string getTypeName(const std::type_index& idx)
  {
    int status = 0;
    auto name = abi::__cxa_demangle(idx.name(), nullptr, nullptr, &status);

    if (name)
    {
      std::string str(name);
      free(name);
      return str;
    }
    else
    {
      std::cerr << "Failed to demangle, status: " << status << std::endl;
      return "N/A";
    }
  }

public:
  template <typename Trafo>
  static void printCapture(const char* expr)
  {
    std::cout << expr << " : " << getTypeName(
      typeid(typename connector<dummy_plan, Trafo>::type)
    ) << std::endl;
  }
};

#define CAPTURE(expr) capture::printCapture<decltype(expr)>(BOOST_PP_STRINGIZE(expr))

BOOST_AUTO_TEST_CASE(SegmentCaptureFunctionPtr)
{
  std::cout << "\n\nCapture function pointers:\n";

  CAPTURE(len1);
  CAPTURE(len2);
  CAPTURE(len3);
}

BOOST_AUTO_TEST_CASE(SegmentCaptureStdFunction)
{
  std::cout << "\n\nCapture std::functions:\n";

  auto oo = std::function<std::size_t(const std::string&)>(len1);
  CAPTURE(oo);

  auto on = std::function<
    void(const std::string&, boost::pipeline::queue_front<std::size_t>&)
  >(len2);
  CAPTURE(on);

  auto nn = std::function<void(
    const boost::pipeline::queue_back<std::string>& in,
    boost::pipeline::queue_front<std::size_t>& out
  )>(len3);
  CAPTURE(nn);
}

BOOST_AUTO_TEST_CASE(SegmentCaptureLambda)
{
  std::cout << "\n\nCapture lambda expressions:\n";

  auto oo = [](const std::string& str) { return str.size(); };
  auto oom = [](const std::string& str) mutable { return str.size(); };

  auto on = [](const std::string& str, boost::pipeline::queue_front<std::size_t>& out)
    {
      out.push_back(str.size());
    }
  ;

  auto nm =
    [](
      const boost::pipeline::queue_back<std::string>& in,
      boost::pipeline::queue_front<std::size_t>& out
    )
    {
      for (const auto& str : in)
      {
        out.push_back(str.size());
      }
    }
  ;

  CAPTURE(oo);
  CAPTURE(oom);
  CAPTURE(on);
  CAPTURE(nm);
}

struct len_t
{
  size_t operator()(const std::string& str)
  {
    return str.size();
  }
};

BOOST_AUTO_TEST_CASE(SegmentCaptureFunctor)
{
  std::cout << "\n\nCapture functor:\n";

  len_t oo_functor;
  CAPTURE(oo_functor);
}

std::size_t len_ntimes(std::size_t n, const std::string& str)
{
  return n * str.size();
}

std::size_t len_min(
  std::size_t min,
  const std::string& str,
  boost::pipeline::queue_front<std::size_t>& out
)
{
  auto len = str.size();
  if (len >= min)
  {
    out.push_back(len);
  }

  return len;
}

std::size_t aggr_len_min(
  std::size_t min,
  boost::pipeline::queue_back<std::string>& in
)
{
  std::size_t out = 0;
  for (const auto& str : in)
  {
    if (str.size() >= min)
    {
      out += str.size();
    }
  }

  return out;
}

std::size_t len_min_ntimes(
  std::size_t min,
  boost::pipeline::queue_back<std::string>& in,
  boost::pipeline::queue_front<std::size_t>& out
)
{
  for (const auto& str : in)
  {
    if (str.size() >= min)
    {
      out.push_back(str.size());
    }
  }

  return 0;
}

BOOST_AUTO_TEST_CASE(SegmentCaptureStdBind)
{
  using namespace std::placeholders;

  std::cout << "\n\nCapture std::bind expressions:\n";

  CAPTURE(std::bind(len_ntimes, 2, _1));
  CAPTURE(std::bind(len_min, 3, _1, _2));
  CAPTURE(std::bind(aggr_len_min, 3, _1));
  CAPTURE(std::bind(len_min_ntimes, 3, _1, _2));
}
