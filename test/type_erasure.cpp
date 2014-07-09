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
//  auto s = from(input) | func | output;

  thread_pool pool{1};
  auto exec = s.run(pool);
  exec.wait();

  std::vector<std::size_t> expected_output{3, 4, 5};

  BOOST_CHECK(output == expected_output);

  (void)s;
}

BOOST_AUTO_TEST_CASE(ClosedOpen)
{
  std::vector<std::string> input;
  std::vector<std::size_t> output;

  segment<terminated, std::size_t> s = from(input) | func;
//  segment<terminated, terminated> s2 = s | output;

  (void)s2;
}

//BOOST_AUTO_TEST_CASE(OpenClosed)
//{
//  std::vector<std::size_t> output;
//
////  segment<std::string, terminated> s = make(func) | output;
//  auto s = make(func) | output;
//
//  (void)s;
//}

//BOOST_AUTO_TEST_CASE(OpenClosedWithTo)
//{
//  segment<std::string, terminated> s = to(func);
//
//  (void)s;
//}

//BOOST_AUTO_TEST_CASE(OpenOpen)
//{
//  segment<std::string, std::size_t> s = make(func);
//
//  (void)s;
//}
