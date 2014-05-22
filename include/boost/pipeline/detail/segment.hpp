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

#include <boost/pipeline/queue.hpp>

namespace boost {
namespace pipeline {
namespace detail {

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
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * transforms each item using `_function`
   * and feeds them into the returned queue.
   */
  queue_back<Output> run()
  {
    auto in_queue = base_segment::_parent.run();
    queue<Output> out_queue;

    for (const auto& in_item : in_queue)
    {
      out_queue.push_back(_function(in_item));
    }

    return out_queue;
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class one_n_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<R(
    const input_type&,
    queue_front<Output>&
  )> function_type;

  one_n_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * transforms each item using `_function`.
   *
   * Also passes the soon to be returned
   * queue to `_function`, it's the transformations
   * responsibility to feed the queue at will.
   */
  queue_back<Output> run()
  {
    auto in_queue = base_segment::_parent.run();
    queue<Output> out_queue;

    for (const auto& in_item : in_queue)
    {
      _function(in_item, out_queue);
    }

    return out_queue;
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output>
class n_one_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<Output(
    queue_back<Output>&
  )> function_type;

  n_one_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * aggregates every entry into a single one
   * using `_function`.
   */
  Output run()
  {
    auto in_queue = base_segment::_parent.run();
    return _function(in_queue);
  }

private:
  function_type _function; /**< transformation function of input */
};

template <typename Parent, typename Output, typename R>
class n_m_segment : public basic_segment<Parent, Output>
{
  typedef basic_segment<Parent, Output> base_segment;

public:
  typedef typename base_segment::value_type value_type;
  typedef typename base_segment::input_type input_type;

  typedef std::function<R(
    queue_back<input_type>&,
    queue_front<Output>&
  )> function_type;

  n_m_segment(
    const Parent& parent,
    const function_type& function
  )
    :base_segment(parent),
     _function(function)
  {}

  /**
   * Runs the segment.
   *
   * Gets the output of the `_parent` segment,
   * and passes the parent queue and the
   * soon to be returned queue to `_function`.
   * It's the transformations responsibility
   * to feed the queue at will.
   */
  queue_back<Output> run()
  {
    auto in_queue = base_segment::_parent.run();
    queue<Output> out_queue;

    _function(in_queue, out_queue);

    return out_queue;
  }

private:
  function_type _function; /**< transformation function of input */
};

//
// is_segment predicate
//

template <typename NotSegment>
struct is_segment : public std::false_type {};

template <typename P, typename O>
struct is_segment<basic_segment<P, O>> : public std::true_type {};

template <typename P, typename O>
struct is_segment<one_one_segment<P, O>> : public std::true_type {};

template <typename P, typename O, typename R>
struct is_segment<one_n_segment<P, O, R>> : public std::true_type {};

template <typename P, typename O>
struct is_segment<n_one_segment<P, O>> : public std::true_type {};

template <typename P, typename O, typename R>
struct is_segment<n_m_segment<P, O, R>> : public std::true_type {};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE__HPP
