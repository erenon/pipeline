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

#include <string>
#include <vector>

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE TypeErasure
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

std::size_t len(const std::string& in) { return in.size(); }

BOOST_AUTO_TEST_CASE(ClosedClosed)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, terminated> s = from(input) | len | output;

  thread_pool pool{1};
  auto exec = s.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

BOOST_AUTO_TEST_CASE(ClosedOpen)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, std::string> s1 = from(input);
  segment<terminated, std::size_t> s2 = s1 | len;
  segment<terminated, terminated>  s3 = s2 | output;

  thread_pool pool{1};
  auto exec = s3.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

BOOST_AUTO_TEST_CASE(OpenOpenClosed)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, std::string> s1 = from(input);
  segment<std::string, std::size_t> s2 = make(len);
  segment<std::string, terminated> s3 = s2 | output;
  segment<terminated, terminated>  s4 = s1 | s3;

  thread_pool pool{1};
  auto exec = s4.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

BOOST_AUTO_TEST_CASE(OpenClosed)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, std::string> s1 = from(input);
  segment<std::string, terminated> s2 = make(len) | output;
  segment<terminated, terminated>  s3 = s1 | s2;

  thread_pool pool{1};
  auto exec = s3.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

std::size_t g_len_sum;

bool sum_len(const std::string& input)
{
  g_len_sum += input.size();
  return true;
}

BOOST_AUTO_TEST_CASE(OpenClosedWithTo)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};

  segment<terminated, std::string> s1 = from(input);
  segment<std::string, terminated> s2 = to(sum_len);
  segment<terminated, terminated>  s3 = s1 | s2;

  g_len_sum = 0;

  thread_pool pool{1};
  auto exec = s3.run(pool);
  exec.wait();

  BOOST_CHECK_EQUAL(g_len_sum, 12u);
}

void execute_plan(plan& p)
{
  thread_pool pool{1};
  auto exec = p.run(pool);
  exec.wait();
}

BOOST_AUTO_TEST_CASE(ExecutePlan)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, std::string> s1 = from(input);
  segment<std::string, terminated> s2 = make(len) | output;
  segment<terminated, terminated>  s3 = s1 | s2;

  execute_plan(s3);

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

segment<terminated, std::size_t> append_len(const segment<terminated, std::string>& s1)
{
  return s1 | len;
}

BOOST_AUTO_TEST_CASE(PassSegmentAround)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, std::string> s1 = from(input);
  segment<terminated, std::size_t> s2 = append_len(s1);
  segment<terminated, terminated>  s3 = s2 | output;

  execute_plan(s3);

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

BOOST_AUTO_TEST_CASE(ClosedOpenTrafo)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<std::string, std::size_t> s2 = make(len);
  segment<terminated, std::string> s1 = from(input);
  segment<terminated, terminated> s = s1 | s2 | output;

  execute_plan(s);
  std::vector<std::size_t> expected_output{3, 4, 5};
  BOOST_CHECK(output == expected_output);
}
