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

namespace boost {
namespace pipeline {
namespace detail {

template <typename Iterator>
class range_reader
{
public:
  typedef decltype(*std::declval<Iterator>()) value_type;

  range_reader(const Iterator& begin, const Iterator& end)
    :_current(begin),
     _end(end)
  {}

  template <typename OutputIt>
  void run(OutputIt output_it)
  {
    while (_current != _end)
    {
      *output_it = *_current;
      ++_current;
      ++output_it;
    }
  }

private:
  Iterator _current;
  Iterator _end;
};

} // namespace detail
} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_RANGE_READER_HPP
