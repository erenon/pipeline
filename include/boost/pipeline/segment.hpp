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

// forward declarations

template <typename Parent, typename Output>
class one_one_segment;

template <typename Parent, typename Output>
class one_n_segment;

// end forward declarations

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
class basic_segment
{
public:
  typedef typename std::remove_reference<
    typename std::remove_reference<Parent>::type::value_type
  >::type input_type;

  typedef Output value_type;

  /**
   * Creates a new segment by concatenating `function` after `parent`.
   *
   * Parent and function_type must be copy constructible
   */
  basic_segment(const Parent& parent)
    :_parent(parent)
  {}

protected:
  Parent _parent;          /**< parent segment, provider of input */
};

template <typename Parent, typename Output>
class one_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<value_type(const input_type&)> function_type;

  one_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Creates a new segment by concatenating `function` after *this
   *
   * Function must be copy constructible. The Output template argument
   * of the returned segment is deduced from the return type of `function`.
   */
  template <typename Function>
  auto operator|(const Function& function)
    -> one_one_segment<decltype(*this), decltype(function(std::declval<value_type>()))>
  {
    return one_one_segment<decltype(*this), decltype(function(std::declval<value_type>()))>(
      *this,
      function
    );
  }

  // TODO take care of std::bind
//  template <typename Function>
//  auto operator|(const Function& function)
//    -> one_n_segment<decltype(*this), typename Function::result_type>
//  {
//    return one_n_segment<decltype(*this), typename Function::result_type>(
//      *this,
//      function
//    );
//  }

  template <typename FInput, typename FOutputIt>
  auto operator|(void function(FInput, FOutputIt&))
    -> one_n_segment<decltype(*this), FOutputIt>
  {
    return one_n_segment<decltype(*this), FOutputIt>(
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

    base_segment::_parent.run(parent_out_it);

    for (auto& input_item : input)
    {
      *out_it = _function(input_item);
      ++out_it;
    }
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename OutputIt>
class one_n_segment : public basic_segment<Parent, typename OutputIt::container_type::value_type>
{
  typedef basic_segment<Parent, typename OutputIt::container_type::value_type> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<void(const input_type&, OutputIt&)> function_type;

  one_n_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  template <typename Function>
  auto operator|(const Function& function)
    -> one_one_segment<decltype(*this), decltype(function(std::declval<value_type>()))>
  {
    return one_one_segment<decltype(*this), decltype(function(std::declval<value_type>()))>(
      *this,
      function
    );
  }

  template <typename FInput, typename FOutputIt>
  auto operator|(void function(FInput, FOutputIt&))
    -> one_n_segment<decltype(*this), FOutputIt>
  {
    return one_n_segment<decltype(*this), FOutputIt>(
      *this,
      function
    );
  }

  void run(OutputIt out_it)
  {
    typedef typename std::decay<
      typename function_type::second_argument_type
    >::type::container_type input_container;

    input_container input;
    auto parent_out_it = std::back_inserter(input);

    base_segment::_parent.run(parent_out_it);

    for (auto& input_item : input)
    {
      _function(input_item, out_it);
    }
  }

private:
  function_type _function; /**< transformation function of input */
};

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE__HPP
