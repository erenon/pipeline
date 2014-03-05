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

#ifndef BOOST_PIPELINE_PIPELINE__HPP
#define BOOST_PIPELINE_PIPELINE__HPP

#include <boost/pipeline/segment.hpp>

namespace boost {
namespace pipeline {

template <typename Container>
segment<Container, typename Container::value_type>
from(Container& container)
{
  typedef typename Container::value_type value_type;
  return segment<Container, value_type>(
    container,
    std::function<value_type(value_type)>()
  );
}

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_PIPELINE__HPP
