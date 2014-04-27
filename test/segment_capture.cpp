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

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE SegmentCapture
#include <boost/test/unit_test.hpp>

class capture
{
  std::string getTypeName(const std::type_index& idx)
  {
    if (idx == std::type_index(typeid(std::size_t)))
    {
      return "std::size_t";
    }
    if (idx == std::type_index(typeid(std::string)))
    {
      return "std::string";
    }
    if (idx == std::type_index(typeid(void)))
    {
      return "void";
    }
    if (idx == std::type_index(typeid(boost::pipeline::queue<std::size_t>)))
    {
      return "boost::pipeline::queue<std::size_t>";
    }
    if (idx == std::type_index(typeid(boost::pipeline::queue<std::string>)))
    {
      return "boost::pipeline::queue<std::string>";
    }
    else
    {
      return idx.name();
    }
  }
protected:
  template <typename R, typename A, typename B>
  void printCapture()
  {
    std::cout
      << "Result: " << getTypeName(typeid(R)) << std::endl
      << "Arg 1 : " << getTypeName(typeid(A)) << std::endl
      << "Arg 2 : " << getTypeName(typeid(B)) << std::endl
    ;
  }
};

class function_capture : private capture
{
  // operator() const, single arg
  template <typename Callable, typename R, typename A>
  void captureMember(R(Callable::*function)(A) const)
  {
    (void) function;

    printCapture<R, A, void>();
  }

  // operator() const, two args
  template <typename Callable, typename R, typename A, typename B>
  void captureMember(R(Callable::*function)(A, B) const)
  {
    (void) function;

    printCapture<R, A, B>();
  }

  // operator() non-const, single arg
  template <typename Callable, typename R, typename A>
  void captureMember(R(Callable::*function)(A))
  {
    (void) function;

    printCapture<R, A, void>();
  }

  // operator() non-const, two args
  template <typename Callable, typename R, typename A, typename B>
  void captureMember(R(Callable::*function)(A))
  {
    (void) function;

    printCapture<R, A, B>();
  }

public:
  // Function pointer, single arg
  template <typename R, typename A>
  void capture(R(function)(A))
  {
    (void) function;

    printCapture<R, A, void>();
  }

  // Functin pointer, two args
  template <typename R, typename A, typename B>
  void capture(R(function)(A, B))
  {
    (void) function;

    printCapture<R, A, B>();
  }

  // std::function, single arg
  template <typename R, typename A>
  void capture(const std::function<R(A)>& function)
  {
    (void) function;

    printCapture<R, A, void>();
  }

  // std::function, two args
  template <typename R, typename A, typename B>
  void capture(const std::function<R(A, B)>& function)
  {
    (void) function;

    printCapture<R, A, B>();
  }

  // has operator()
  template <typename Callable>
  void capture(const Callable& lambda)
  {
    (void) lambda;

    captureMember(&Callable::operator());
  }
};

std::size_t len1(const std::string& str)
{
  return str.size();
}

void len2(const std::string& str, boost::pipeline::queue<std::size_t>& out)
{
  auto len = str.size();
  if (len >= 3)
  {
    out.push_back(len);
  }
}

void len3(
  const boost::pipeline::queue<std::string>& in,
  boost::pipeline::queue<std::size_t>& out
)
{
  for (const auto& str : in)
  {
    out.push_back(str.size());
  }
}

BOOST_AUTO_TEST_CASE(SegmentCaptureFunctionPtr)
{
  std::cout << "\n\nCapture function pointers:\n";

  function_capture smt;

  smt.capture(len1);
  smt.capture(len2);
  smt.capture(len3);
}

BOOST_AUTO_TEST_CASE(SegmentCaptureStdFunction)
{
  std::cout << "\n\nCapture std::functions:\n";

  function_capture smt;

  auto f1 = std::function<std::size_t(const std::string&)>(len1);
  smt.capture(f1);

  auto f2 = std::function<void(const std::string&, boost::pipeline::queue<std::size_t>&)>(len2);
  smt.capture(f2);

  auto f3 = std::function<void(
    const boost::pipeline::queue<std::string>& in,
    boost::pipeline::queue<std::size_t>& out
  )>(len3);
  smt.capture(f3);
}

BOOST_AUTO_TEST_CASE(SegmentCaptureLambda)
{
  std::cout << "\n\nCapture lambda expressions:\n";

  function_capture smt;

  smt.capture([](const std::string& str) { return str.size(); });
  smt.capture([](const std::string& str) mutable { return str.size(); });

  smt.capture([](const std::string& str, boost::pipeline::queue<std::size_t>& out)
    {
      out.push_back(str.size());
    }
  );

  smt.capture([](const boost::pipeline::queue<std::string>& in, boost::pipeline::queue<std::size_t>& out)
    {
      for (const auto& str : in)
      {
        out.push_back(str.size());
      }
    }
  );
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

  function_capture smt;

  len_t functor;
  smt.capture(functor);
}

template <typename Value>
class bind_capture : private capture
{
  template <typename Bind, typename R>
  class is_one_one_trafo
  {
    template <
      typename Bind1,
      typename R1,
      typename std::enable_if<std::is_same<
        R1,
        decltype(std::declval<Bind1>()(
          std::declval<Value>()
        ))
      >::value, int>::type = 0
    >
    static int test(int);

    template <typename Bind1, typename R1>
    static long test(...);

  public:
    enum { value = std::is_same<int, decltype(test<Bind, R>(0))>::value };
  };

  template <typename Bind, typename R>
  class is_n_one_trafo
  {
    template <
      typename Bind1,
      typename R1,
      typename std::enable_if<std::is_same<
        R1,
        decltype(std::declval<Bind1>()(
          std::declval<boost::pipeline::queue<Value>&>()
        ))
      >::value, int>::type = 0
    >
    static int test(int);

    template <typename Bind1, typename R1>
    static long test(...);

  public:
    enum { value = std::is_same<int, decltype(test<Bind, R>(0))>::value };
  };

  // operator() template, 1-1
  template <typename Bind, typename R>
  typename std::enable_if<
    is_one_one_trafo<Bind, R>::value
  >::type
  captureCallOpTemplate()
  {
    printCapture<R, Value, void>();
  }

  // operator() template, 1-N
  template <typename Bind, typename R>
  typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind>()(
        std::declval<Value>(),
        std::declval<boost::pipeline::queue<R>&>()
      ))
    >::value
  && ! is_one_one_trafo<Bind, R>::value
  >::type
  captureCallOpTemplate()
  {
    printCapture<R, Value, boost::pipeline::queue<R>&>();
  }

  // operator() template, N-1
  template <typename Bind, typename R>
  typename std::enable_if<
    is_n_one_trafo<Bind, R>::value
  >::type
  captureCallOpTemplate()
  {
    printCapture<R, boost::pipeline::queue<Value>&, void>();
  }

  // operator() template, N-M
  template <typename Bind, typename R>
  typename std::enable_if<
    std::is_same<
      R,
      decltype(std::declval<Bind>()(
        std::declval<boost::pipeline::queue<Value>&>(),
        std::declval<boost::pipeline::queue<R>&>()
      ))
    >::value
  && ! is_n_one_trafo<Bind, R>::value
  >::type
  captureCallOpTemplate()
  {
    printCapture<R, boost::pipeline::queue<Value>&, boost::pipeline::queue<R>&>();
  }

public:
  template <typename Bind>
  typename std::enable_if<std::is_bind_expression<Bind>::value>::type
  capture(const Bind& binding)
  {
    (void) binding;

    captureCallOpTemplate<Bind, typename Bind::result_type>();
  }
};

std::size_t len_ntimes(std::size_t n, const std::string& str)
{
  return n * str.size();
}

std::size_t len_min(
  std::size_t min,
  const std::string& str,
  boost::pipeline::queue<std::size_t>& out
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
  boost::pipeline::queue<std::string>& in
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
  boost::pipeline::queue<std::string>& in,
  boost::pipeline::queue<std::size_t>& out
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

  bind_capture<std::string> smt;

  smt.capture(std::bind(len_ntimes, 2, _1));
  smt.capture(std::bind(len_min, 3, _1, _2));
  smt.capture(std::bind(aggr_len_min, 3, _1));
  smt.capture(std::bind(len_min_ntimes, 3, _1, _2));
}
