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
  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | atoi
    | nums_out
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
}

void keep_and_twice(
  const int& item,
  boost::pipeline::queue_back<int>& out
) {
  out.push(item);
  out.push(item * 2);
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

  BOOST_CHECK_EQUAL(nums_out.size(), 10);
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

void sum_diff_prod(
  boost::pipeline::queue_front<int>& in,
  boost::pipeline::queue_back<int>& out
)
{
  int a;
  int b;

  if (in.wait_pull(a) && in.wait_pull(b))
  {
    out.push(a+b);
    out.push(a-b);
    out.push(a*b);
  }
}

BOOST_AUTO_TEST_CASE(SegmentNToMTrafo)
{
  std::vector<int> nums = {0, 1, 2, 3};
  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | sum_diff_prod
    | nums_out
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(nums_out.size(), 6);
  BOOST_CHECK_EQUAL(nums_out[0], 1);
  BOOST_CHECK_EQUAL(nums_out[1], -1);
  BOOST_CHECK_EQUAL(nums_out[2], 0);
  BOOST_CHECK_EQUAL(nums_out[3], 5);
  BOOST_CHECK_EQUAL(nums_out[4], -1);
  BOOST_CHECK_EQUAL(nums_out[5], 6);
}

void generate_ints(queue_back<int>& qb)
{
  for (int i = 0; i < 5; ++i)
  {
    qb.push(i);
  }
}

void generate_ints_from(queue_back<int>& qb, const int start)
{
  for (int i = start; i < 5; ++i)
  {
    qb.push(i);
  }
}

template <typename Callable>
void generated_segment_test(Callable& generator)
{
  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(generator)
    | nums_out
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(nums_out.size(), 5);
  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
}

BOOST_AUTO_TEST_CASE(GeneratedSegmentFp)
{
  generated_segment_test(generate_ints);
}

BOOST_AUTO_TEST_CASE(GeneratedSegmentFunction)
{
  std::function<void(queue_back<int>&)> gen = generate_ints;
  generated_segment_test(gen);
}

template <typename Hint, typename Callable>
void generated_segment_test_hinted(Callable& generator)
{
  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from<Hint>(generator)
    | nums_out
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(nums_out.size(), 5);
  BOOST_CHECK_EQUAL(nums_out[0], 0);
  BOOST_CHECK_EQUAL(nums_out[1], 1);
  BOOST_CHECK_EQUAL(nums_out[2], 2);
  BOOST_CHECK_EQUAL(nums_out[3], 3);
  BOOST_CHECK_EQUAL(nums_out[4], 4);
}

BOOST_AUTO_TEST_CASE(GeneratedSegmentLambda)
{
  auto gen = [] (queue_back<int> qb)
  {
    for (int i = 0; i < 5; ++i)
    {
      qb.push(i);
    }
  };

  generated_segment_test_hinted<int>(gen);
}

BOOST_AUTO_TEST_CASE(GeneratedSegmentBind)
{
  auto gen = std::bind(generate_ints_from, std::placeholders::_1, 0);
  generated_segment_test_hinted<int>(gen);
}

int consume_sum;

void consume(int input)
{
  consume_sum += input;
}

BOOST_AUTO_TEST_CASE(SegmentProceduralSingleConsumer)
{
  std::vector<int> nums = {0, 1, 2, 3};
  consume_sum = 0;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | consume
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(consume_sum, 6);
}

void consume_n(boost::pipeline::queue_front<int>& qf)
{
  int input;
  while (qf.wait_pull(input))
  {
    consume_sum += input;
  }
}

BOOST_AUTO_TEST_CASE(SegmentProceduralMultiConsumer)
{
  std::vector<int> nums = {0, 1, 2, 3};
  consume_sum = 0;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | consume_n
  ).run(pool);

  exec.wait();

  BOOST_CHECK_EQUAL(consume_sum, 6);
}

int sum_two(boost::pipeline::queue_front<int> in) {
  int a;
  if ( ! in.wait_pull(a))
  {
    return -1;
  }

  int b;
  if ( ! in.wait_pull(b))
  {
    return a;
  }

  return a + b;
}

BOOST_AUTO_TEST_CASE(SegmentNToOneTrafo)
{
  std::vector<int> nums = {0, 1, 2, 3, 4, 5};
  std::vector<int> nums_out;

  thread_pool pool{1};

  auto exec =
  (boost::pipeline::from(nums)
    | sum_two
    | nums_out
  ).run(pool);

  exec.wait();

  std::vector<int> expected_out = {1, 5, 9};

  BOOST_CHECK(expected_out == nums_out);
}
