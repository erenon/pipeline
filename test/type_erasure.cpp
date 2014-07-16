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

std::size_t func(const std::string& in) { return in.size(); }

BOOST_AUTO_TEST_CASE(ClosedClosed)
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  segment<terminated, terminated> s = from(input) | func | output;

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
  segment<terminated, std::size_t> s2 = s1 | func;
  segment<terminated, terminated>  s3 = s2 | output;

  thread_pool pool{1};
  auto exec = s3.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);
}

// TODO type_erasure test incomplete because open_segment bug
//BOOST_AUTO_TEST_CASE(OpenClosed)
//{
//  std::vector<std::string> input;
//  std::vector<std::size_t> output;
//
//  segment<terminated, std::string> s1 = from(input);
//  segment<std::string, terminated> s2 = make(func) | output;
//  segment<terminated, terminated>  s3 = s1 | s2;
//
//  (void)s3;
//}

// TODO type_erasure test incomplete becasue segment<I,O>
// does not model closed_segment
//BOOST_AUTO_TEST_CASE(OpenClosedWithTo)
//{
//  std::vector<std::string> input;
//
//  segment<terminated, std::string> s1 = from(input);
//  segment<std::string, terminated> s2 = to(func);
//  segment<terminated, terminated>  s3 = s1 | s2;
//
//  (void)s3;
//}

BOOST_AUTO_TEST_CASE(OpenOpen)
{
  segment<std::string, std::size_t> s = make(func);

  (void)s;
}
