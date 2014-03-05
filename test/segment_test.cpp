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
#include <functional>

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE SegmentTest
#include <boost/test/unit_test.hpp>

constexpr int twice(const int& input)
{
  return 2 * input;
}

constexpr int add(const int& a, const int& b)
{
  return a + b;
}

BOOST_AUTO_TEST_CASE(SegmentTest)
{
  std::vector<int> nums = {0, 1, 2, 3};

  std::vector<int> nums_out;
  auto out_it = std::back_inserter(nums_out);

  auto add_2 = std::bind(add, 2, std::placeholders::_1);

  (boost::pipeline::from(nums)
    | twice
    | add_2
    | [] (const int& input) { return 10 * input; }
  ).run(out_it);

  BOOST_CHECK_EQUAL(nums_out[0], 20);
  BOOST_CHECK_EQUAL(nums_out[1], 40);
  BOOST_CHECK_EQUAL(nums_out[2], 60);
  BOOST_CHECK_EQUAL(nums_out[3], 80);
}

BOOST_AUTO_TEST_CASE(SegmentPreallocTest)
{
  std::vector<int> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  std::vector<int> nums_out(10, -1);

  (boost::pipeline::from(nums)
    | twice
  ).run(nums_out.begin());

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 2);
  BOOST_CHECK_EQUAL(nums_out[2], 4);
  BOOST_CHECK_EQUAL(nums_out[3], 6);
  BOOST_CHECK_EQUAL(nums_out[4], 8);
  BOOST_CHECK_EQUAL(nums_out[5], 10);
  BOOST_CHECK_EQUAL(nums_out[6], 12);
  BOOST_CHECK_EQUAL(nums_out[7], 14);
  BOOST_CHECK_EQUAL(nums_out[8], 16);
  BOOST_CHECK_EQUAL(nums_out[9], 18);
}
