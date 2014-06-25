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
#include <functional>
#include <cstdlib>
#include <deque>
#include <algorithm>

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE SegmentTest
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

constexpr int twice(const int& input)
{
  return 2 * input;
}

constexpr int add(const int& a, const int& b)
{
  return a + b;
}

BOOST_AUTO_TEST_CASE(SegmentTestInOut)
{
  std::vector<int> nums(1025, 1);

  int n(0);
  std::generate(nums.begin(), nums.end(), [&]{ return n++; });

  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec = (from(nums) | nums_out).run(pool);

  exec.wait();

  BOOST_CHECK(nums == nums_out);
}

BOOST_AUTO_TEST_CASE(SegmentTestOneToOne)
{
  std::vector<int> nums = {0, 1, 2, 3};
  std::vector<int> nums_out;

  auto add_2 = std::bind(add, 2, std::placeholders::_1);

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | twice
    | add_2
    | [] (const int& input) { return 10 * input; }
    | nums_out
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(nums_out[0], 20);
  BOOST_CHECK_EQUAL(nums_out[1], 40);
  BOOST_CHECK_EQUAL(nums_out[2], 60);
  BOOST_CHECK_EQUAL(nums_out[3], 80);
}

BOOST_AUTO_TEST_CASE(SegmentTypeCrossing)
{
  std::vector<const char*> nums{"0", "1", "2", "3", "4"};

  auto nums_out =
  (boost::pipeline::from(nums)
    | atoi
  ).run();

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
}

void keep_and_twice(
  const int& item,
  boost::pipeline::queue_front<int>& out
) {
  out.try_push(item);
  out.try_push(item * 2);
}

BOOST_AUTO_TEST_CASE(SegmentOneToNTrafo)
{
  std::vector<int> nums = {0, 1, 2, 3, 4};
  std::vector<int> nums_out;

  auto add_2 = std::bind(add, 2, std::placeholders::_1);

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | keep_and_twice
    | add_2
    | nums_out
  ).run(pool);

  exec.wait();

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

//int sum_two(boost::pipeline::queue_back<int>& in)
//{
//}
//
//BOOST_AUTO_TEST_CASE(SegmentNToOneTrafo)
//{
//
//}

//void grep(
//  const std::string& equals_to,
//  const std::string& item,
//  boost::pipeline::queue_front<std::string>& out
//)
//{
//  if (equals_to == item)
//  {
//    out.push_back(item);
//  }
//}
//
//BOOST_AUTO_TEST_CASE(SegmentOneToNTrafoBind)
//{
//  std::deque<std::string> strs = {"a", "b", "a", "a", "c"};
//
//  std::string pattern("a");
//
//  auto grepA = std::bind(grep, pattern,
//    std::placeholders::_1, std::placeholders::_2);
//
//  auto strs_out =
//  (boost::pipeline::from(strs)
//    | grepA
//  ).run();
//
//  BOOST_CHECK_EQUAL(strs_out[0], pattern);
//  BOOST_CHECK_EQUAL(strs_out[1], pattern);
//  BOOST_CHECK_EQUAL(strs_out[2], pattern);
//}
