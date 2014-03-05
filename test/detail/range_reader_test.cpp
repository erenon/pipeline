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

#include <vector>
#include <iterator>

#define BOOST_TEST_MODULE RangeReaderTest
#include <boost/test/unit_test.hpp>

#include <boost/pipeline.hpp>

BOOST_AUTO_TEST_CASE(RangeReaderVectorTest)
{
  using boost::pipeline::detail::range_reader;

  std::vector<int> nums = {0, 1, 2, 3};

  range_reader<std::vector<int>::iterator> range(nums.begin(), nums.end());

  std::vector<int> nums_out;
  auto out_it = std::back_inserter(nums_out);

  range.run(out_it);

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
}

BOOST_AUTO_TEST_CASE(RangeReaderIntArrayTest)
{
  using boost::pipeline::detail::range_reader;

  int nums[] = {0, 1, 2, 3};

  range_reader<int*> range(nums, nums + sizeof(nums)/sizeof(int));

  int nums_out[] = {-1, -1, -1, -1};
  range.run(nums_out);

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
}
