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

#ifndef BOOST_PIPELINE_PIPELINE_HPP
#define BOOST_PIPELINE_PIPELINE_HPP

#include <type_traits>

#include <boost/pipeline/detail/segment.hpp>
#include <boost/pipeline/detail/operator.hpp>
#include <boost/pipeline/detail/open_segment.hpp>
#include <boost/pipeline/detail/closed_segment.hpp>

namespace boost {
namespace pipeline {

/**
 * Creates a range_input_segment operating on `container`.
 */
template <typename Container>
detail::range_input_segment<typename Container::iterator>
from(Container& container)
{
  typedef detail::range_input_segment<typename Container::iterator> c_input_segment;

  return c_input_segment(container.begin(), container.end());
}

/**
 * Creates a range_input_segment operating on a range.
 */
template <typename Iterator>
detail::range_input_segment<Iterator>
from(const Iterator& begin, const Iterator& end)
{
  typedef detail::range_input_segment<Iterator> range_input_segment;

  return range_input_segment(begin, end);
}

/**
 * Creates a generator_input_segment operating on values
 * produced by a generator, generator is an std::function
 */
template <typename QueueBack, typename R>
detail::generator_input_segment<
  std::function<R(QueueBack)>,
  typename std::remove_reference<QueueBack>::type::value_type
>
from(const std::function<R(QueueBack)>& generator)
{
  typedef detail::generator_input_segment<
    std::function<R(QueueBack)>,
    typename std::remove_reference<QueueBack>::type::value_type
  > input_segment;

  return input_segment(generator);
}

/**
 * Creates a generator_input_segment operating on values
 * produced by a generator, generator is a function pointer
 */
template <typename QueueBack, typename R>
detail::generator_input_segment<
  R(*)(QueueBack),
  typename std::remove_reference<QueueBack>::type::value_type
>
from(R(*generator)(QueueBack))
{
  typedef detail::generator_input_segment<
    R(*)(QueueBack),
    typename std::remove_reference<QueueBack>::type::value_type
  > input_segment;

  return input_segment(generator);
}

/**
 * Creates a queue_input_segment representing `queue`
 */
template <typename T>
detail::queue_input_segment<T>
from(queue<T>& queue)
{
  return detail::queue_input_segment<T>(queue);
}

/**
 * Creates a generator_input_segment operating on values
 * produced by a generator, generator is callable:
 * generator(queue_back<T>);
 *
 * The purpose of this overload is to support lambdas,
 * bind expressions and functors.
 *
 * Example:
 * from<int>([](queue_back<int>& qb) {...});
 *
 * TODO lambda and functor do not require a hint,
 * bind does.
 */
template <typename T, typename Callable>
detail::generator_input_segment<Callable, T>
from(const Callable& generator)
{
  typedef detail::generator_input_segment<Callable, T> input_segment;

  return input_segment(generator);
}

/**
 * Creates on open_segment representing `function`
 */
template <typename Function, typename std::enable_if<
  ! std::is_function<Function>::value
,int>::type = 0>
detail::open_segment<const Function&>
make(const Function& function)
{
  return detail::open_segment<const Function&>(function);
}

/**
 * Creates on open_segment representing `function`
 */
template <typename Function, typename std::enable_if<
  ! std::is_function<Function>::value
,int>::type = 0>
detail::open_segment<Function&>
make(Function& function)
{
  return detail::open_segment<Function&>(function);
}

/**
 * Creates on open_segment representing `function`,
 * when `function` is of a function type.
 */
template <typename Function, typename std::enable_if<
  std::is_function<Function>::value
,int>::type = 0>
detail::open_segment<typename std::add_pointer<Function>::type>
make(const Function& function)
{
  return detail::open_segment<typename std::add_pointer<Function>::type>(&function);
}

/**
 * Creates a right-terminated *_output_segment
 * which encapsulates `consumer`
 */
template <typename Callable, typename std::enable_if<
! std::is_function<Callable>::value
,int>::type = 0>
detail::closed_segment<Callable>
to(const Callable& consumer)
{
  return detail::closed_segment<Callable>{consumer};
}

/**
 * Creates a right-terminated *_output_segment
 * which encapsulates `consumer`.
 *
 * `consumer` is of a function type.
 */
template <typename Function, typename std::enable_if<
  std::is_function<Function>::value
,int>::type = 0>
detail::closed_segment<typename std::add_pointer<Function>::type>
to(const Function& consumer)
{
  return detail::closed_segment<typename std::add_pointer<Function>::type>{&consumer};
}

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_PIPELINE_HPP
