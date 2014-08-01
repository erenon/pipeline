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
#include <iostream>

#include <boost/algorithm/string/trim.hpp>

#include <boost/pipeline.hpp>
namespace ppl = boost::pipeline;

std::size_t length(const std::string& input)
{
  return input.size();
}

std::string trim(const std::string& input)
{
  return boost::algorithm::trim_copy(input);
}

void use_auto()
{
  std::vector<std::string> lines;
  std::vector<std::size_t> output;

  //[example_open_segment_auto
  auto s2 = ppl::make(length) | output;
  auto s1 = ppl::from(lines) | trim;
  auto s  = s1 | s2;
  //]

  (void)s;
}

// TODO example open_segment type erasure disabled
/*
void use_type_erasure()
{
  std::vector<std::string> lines;
  std::vector<std::size_t> output;

  //[example_open_segment_type_erasure
  ppl::segment<std::string, std::size_t> s2 = ppl::make(length);
  ppl::segment<std::string, std::string> s1 = ppl::from(lines) | trim;
  ppl::segment<ppl::teminated, ppl::teminated> s = s1 | s2 | output;
  //]
}
*/

int main()
{
  use_auto();
}
