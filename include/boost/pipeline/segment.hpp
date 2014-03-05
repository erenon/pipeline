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

#ifndef BOOST_PIPELINE__HPP
#define BOOST_PIPELINE__HPP

#include <iterator>
#include <functional>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <utility>

namespace boost {
namespace pipeline {

template <typename S, typename Param>
class is_runnable
{
  template <typename S1, typename Param1>
  static decltype(std::declval<S1>().run(std::declval<Param1>())) get(int);

  template <typename S1, typename Param1>
  static long get(...);

public:
  enum { value = std::is_void<decltype(get<S, Param>(0))>::value };
};

template <typename S>
class is_container
{
  template <typename S1>
  static std::pair<
    decltype(std::declval<S1>().begin()),
    decltype(std::declval<S1>().end()  )
  > get(int);

  template <typename S1>
  static void get(...);

public:
  enum { value = ! std::is_void<decltype(get<S>(0))>::value };
};

template <typename Parent, typename Output>
class segment
{
private:

  typedef segment<Parent, Output> self_type;

public:
  typedef typename std::remove_reference<Parent>::type::value_type input_type;
  typedef Output value_type;
  typedef std::function<value_type(input_type)> function_type;

  segment(
    Parent& parent,
    function_type function
  )
    :_parent(parent),
     _function(function)
  {}

  template<typename ChildOutput>
  auto operator|(std::function<ChildOutput(value_type)>& function)
    -> segment<decltype(*this), ChildOutput>
  {
    return segment<decltype(*this), ChildOutput>(*this, function);
  }

  template<typename ChildOutput>
  auto operator|(ChildOutput function(const value_type&))
    -> segment<decltype(*this), ChildOutput>
  {
    return segment<decltype(*this), ChildOutput>(
      *this,
      std::function<ChildOutput(value_type)>(function)
    );
  }

  template <typename OutputIt>
  void run(OutputIt out_it)
  {
    run_parent<OutputIt, Parent>(out_it);
  }

private:
  template <
    typename OutputIt,
    typename Parent1,
    typename std::enable_if<
      is_runnable<Parent1, OutputIt>::value,
      int
    >::type = 0
  >
  void run_parent(OutputIt out_it)
  {
    std::vector<input_type> input;
    auto parent_out_it = std::back_inserter(input);

    _parent.run(parent_out_it);

    for (auto& input_item : input)
    {
      out_it = _function(input_item);
      ++out_it;
    }
  }

  template <
    typename OutputIt,
    typename Parent1,
    typename std::enable_if<
      is_container<Parent1>::value,
      int
    >::type = 0
  >
  void run_parent(OutputIt out_it)
  {
    for (auto& output_item : _parent)
    {
      out_it = output_item;
      ++out_it;
    }
  }

  Parent& _parent;
  function_type _function;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE__HPP
