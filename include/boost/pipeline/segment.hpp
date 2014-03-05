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

namespace boost {
namespace pipeline {

template <typename Parent, typename Output>
class segment
{
private:
  typedef segment<Parent, Output> self_type;

public:
  typedef typename std::remove_reference<Parent>::type::value_type input_type;
  typedef Output value_type;
  typedef std::function<value_type(const input_type&)> function_type;

  segment(
    const Parent& parent,
    const function_type& function
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
    std::vector<input_type> input;
    auto parent_out_it = std::back_inserter(input);

    _parent.run(parent_out_it);

    for (auto& input_item : input)
    {
      out_it = _function(input_item);
      ++out_it;
    }
  }

  Parent _parent;
  function_type _function;
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE__HPP
