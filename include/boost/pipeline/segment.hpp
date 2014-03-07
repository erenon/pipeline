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

/**
 * Represents a series of connected operations.
 *
 * Template arguments:
 *
 *  - Parent: type of the segment before this segment in the pipline;
 *    in case of (foo | bar | baz) baz has a type of
 *    segment<segment<segment<?, Out>, Out> Out>
 *
 *    Parent must have a run<OutIt>(OutIt) method and value_type typedef.
 *    The method run feeds instances of value_type into a range
 *    pointed by the given iterator.
 *
 *  - Output: type of emitted entries
 */
template <typename Parent, typename Output>
class segment
{
public:
  typedef typename std::remove_reference<
    typename std::remove_reference<Parent>::type::value_type
  >::type input_type;

  typedef Output value_type;
  typedef std::function<value_type(const input_type&)> function_type;

  /**
   * Creates a new segment by concatenating `function` after `parent`.
   *
   * Parent and function_type must be copy constructible
   */
  segment(
    const Parent& parent,
    const function_type& function
  )
    :_parent(parent),
     _function(function)
  {}

  /**
   * Creates a new segment by concatenating `function` after *this
   *
   * Function must be copy constructible. The Output template argument
   * of the returned segment is deduced from the return type of `function`.
   */
  template<typename Function>
  auto operator|(const Function& function)
    -> segment<decltype(*this), decltype(function(std::declval<value_type>()))>
  {
    return segment<decltype(*this), decltype(function(std::declval<value_type>()))>(
      *this,
      function
    );
  }

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * transforms each entry using `_function`
   * and feeds them into `out_it`.
   */
  template <typename OutputIt>
  void run(OutputIt out_it)
  {
    std::vector<input_type> input;
    auto parent_out_it = std::back_inserter(input);

    _parent.run(parent_out_it);

    for (auto& input_item : input)
    {
      *out_it = _function(input_item);
      ++out_it;
    }
  }

private:
  Parent _parent;          /**< parent segment, provider of input */
  function_type _function; /**< transformation function of input */
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE__HPP
