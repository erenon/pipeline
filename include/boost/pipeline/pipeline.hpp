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
 * Creates a segment operating on `container`.
 *
 * Join other transformations (segments) using the pipe operator
 * to create a pipeline.
 *
 * @param container Container of items which form the input of the pipeline
 *
 * @returns `segment<terminated, T>`, `T` is the `value_type` of `Container`
 */
template <typename Container>
detail::range_input_segment<typename Container::const_iterator>
from(const Container& container)
{
  typedef detail::range_input_segment<typename Container::const_iterator> c_input_segment;

  return c_input_segment(container.cbegin(), container.cend());
}

/**
 * Creates a segment operating on a range.
 *
 * Join other transformations (segments) using the pipe operator
 * to create a pipeline.
 *
 * @param begin Beginning of the input range
 * @param end End of the input range (exclusive)
 *
 * @returns `segment<terminated, T>`, `T` is the `value_type` of `Iterator`
 */
template <typename Iterator>
detail::range_input_segment<Iterator>
from(const Iterator& begin, const Iterator& end)
{
  typedef detail::range_input_segment<Iterator> range_input_segment;

  return range_input_segment(begin, end);
}

/**
 * Creates a segment operating on items
 * produced by a generator function.
 *
 * The generator function receives a `queue_back<T>` argument
 * and feeds it with the generated items. The underlying queue
 * will be automatically closed upon return of the generator.
 *
 * @param generator An `std::function` receiving a single `queue_back<T>` argument.
 * @returns `segment<terminated, T>`
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
 * Creates a segment operating on items
 * produced by a generator function.
 *
 * The generator function receives a `queue_back<T>` argument
 * and feeds it with the generated items. The underlying queue
 * will be automatically closed upon return of the generator.
 *
 * @param generator The pointed function is receiving a single `queue_back<T>` argument.
 * @returns `segment<terminated, T>`
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
 * Creates a segment operating on items
 * produced by a generator function.
 *
 * The generator function receives a `queue_back<T>` argument
 * and feeds it with the generated items. The underlying queue
 * will be automatically closed upon return of the generator.
 *
 * This overload accepts lambdas,
 * bind expressions and functors.
 *
 * To deduce the `value_type` of the generator, a hint might be required:
 *
 * @code
 * from<int>([](queue_back<int>& qb) {...});
 * @endcode
 *
 * @param generator A callable which is receiving a single `queue_back<T>` argument.
 * @returns `segment<terminated, T>`
 *
 * @b TODO lambda and functor should not require a hint.
 */
template <typename T, typename Callable>
detail::generator_input_segment<Callable, T>
from(const Callable& generator)
{
  typedef detail::generator_input_segment<Callable, T> input_segment;

  return input_segment(generator);
}

/**
 * Creates a segment operating on items of the given `queue`.
 *
 * Elements to process might be added to the queue later
 * until the queue is closed by the application.
 *
 * @param queue Queue containing the input of the pipeline
 * @returns `segment<terminated, T>`, `T` is `value_type` of `queue`
 */
template <typename T>
detail::queue_input_segment<T>
from(queue<T>& queue)
{
  return detail::queue_input_segment<T>(queue);
}

/**
 * Creates an open, non-terminated segment representing
 * `function`.
 *
 * @param function Arbitrary transformation, but not a function pointer
 * @returns `segment<unknown, T>`
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
 * Creates an open, non-terminated segment representing
 * `function`.
 *
 * Taking the transformation as a non-const reference
 * makes it possible to chain containers as sinks.
 *
 * @param function Arbitrary transformation, but not a function pointer
 * @returns `segment<unknown, T>`
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
 * Creates an open, non-terminated segment representing
 * `function`.
 *
 * @param function A transformation pointed by a function pointer
 * @returns `segment<unknown, T>`
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
 * Creates a right-terminated segment representing `consumer`
 *
 * Wrap sink transformations returning non-void by this
 * call to create a right-terminated segment.
 *
 * @param consumer Sink transformation, non-function pointer
 * @returns `segment<unknown, terminated>`
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
 * Creates a right-terminated segment representing `consumer`
 *
 * Wrap sink transformations returning non-void by this
 * call to create a right-terminated segment.
 *
 * @param consumer Sink transformation, function pointer
 * @returns `segment<unknown, terminated>`
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
