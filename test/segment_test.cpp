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
#include <cstdlib>
#include <deque>

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

BOOST_AUTO_TEST_CASE(SegmentTypeCrossing)
{
  std::vector<const char*> nums{{"0"}, {"1"}, {"2"}, {"3"}, {"4"}};
  std::vector<int> nums_out(5, -1);

  (boost::pipeline::from(nums)
    | atoi
  ).run(nums_out.begin());

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
}

void keep_and_twice(
  const int& item,
  std::back_insert_iterator<std::vector<int>>& it
) {
  *it = item;
  ++it;
  *it = item * 2;
  ++it;
}

BOOST_AUTO_TEST_CASE(SegmentOneToNTrafo)
{
  std::vector<int> nums = {0, 1, 2, 3, 4};
  std::vector<int> nums_out(10, -1);

  auto add_2 = std::bind(add, 2, std::placeholders::_1);

  (boost::pipeline::from(nums)
    | keep_and_twice
    | add_2
  ).run(nums_out.begin());

  BOOST_CHECK_EQUAL(nums_out[0], 2);
  BOOST_CHECK_EQUAL(nums_out[1], 2);
  BOOST_CHECK_EQUAL(nums_out[2], 3);
  BOOST_CHECK_EQUAL(nums_out[3], 4);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
  BOOST_CHECK_EQUAL(nums_out[5], 6);
  BOOST_CHECK_EQUAL(nums_out[6], 5);
  BOOST_CHECK_EQUAL(nums_out[7], 8);
  BOOST_CHECK_EQUAL(nums_out[8], 6);
  BOOST_CHECK_EQUAL(nums_out[9], 10);
}

/*std::back_insert_iterator<std::deque<std::string>>*/
void
grep(
  const std::string& equals_to,
  const std::string& item,
  std::back_insert_iterator<std::deque<std::string>>& it
)
{
  if (equals_to == item)
  {
    *it = item;
    ++it;
  }

  //return it;
}

BOOST_AUTO_TEST_CASE(SegmentOneToNTrafoBind)
{
  std::deque<std::string> strs = {"a", "b", "a", "a", "c"};
  std::deque<std::string> strs_out(5);

  std::string pattern("a");

  auto grepA = std::bind(grep, pattern,
    std::placeholders::_1, std::placeholders::_2);

  (boost::pipeline::from(strs)
    | grep
  ).run(strs_out.begin());

  BOOST_CHECK_EQUAL(strs_out[0], pattern);
  BOOST_CHECK_EQUAL(strs_out[1], pattern);
  BOOST_CHECK_EQUAL(strs_out[2], pattern);
}
