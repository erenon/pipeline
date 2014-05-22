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

#ifndef BOOST_PIPELINE_DETAIL_RANGE_READER_HPP
#define BOOST_PIPELINE_DETAIL_RANGE_READER_HPP

#include <type_traits>
#include <algorithm>

#include <boost/pipeline/queue.hpp>

namespace boost {
namespace pipeline {
namespace detail {

/**
 * Represents a range denoted by two iterators.
 *
 * Purpose of this class is to feed a range to a pipeline.
 */
template <typename Iterator>
class range_reader
{
public:
  typedef typename std::remove_reference<
    decltype(*std::declval<Iterator>())
  >::type value_type;

  range_reader(const Iterator& begin, const Iterator& end)
    :_current(begin),
     _end(end)
  {}

  queue_back<value_type> run()
  {
    queue<value_type> q;

    auto output_it = std::back_inserter(q);
    std::copy(_current, _end, output_it);

    return q;
  }

private:
  Iterator _current;
  Iterator _end;
};

//
// is_range_reader predicate
//

template <typename NotRangeReader>
struct is_range_reader : public std::false_type {};

template <typename Iterator>
struct is_range_reader<range_reader<Iterator>> : public std::true_type {};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_RANGE_READER_HPP
