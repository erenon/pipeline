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

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE ItemTypeRequirements
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

class item_type
{
public:
  item_type() = default; // default constructible
  item_type(item_type&&) = default; // movable
  item_type& operator=(item_type&&) = default; // move assignable

  item_type(const item_type&) = delete; // no copy
  item_type& operator=(const item_type&) = delete; // no assignment

  int value = 1;
};

item_type id(const item_type& input)
{
  (void) input;
  return item_type();
}

BOOST_AUTO_TEST_CASE(ReadTransformWrite)
{
  queue<item_type> input;
  queue<item_type> output;

  thread_pool pool{1};
  auto exec = (from(input) | id | output).run(pool);

  input.close();
  exec.wait();
}
