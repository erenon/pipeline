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

void use_type_erasure()
{
  std::vector<std::string> lines;
  std::vector<std::size_t> output;

  //[example_open_segment_type_erasure
  ppl::segment<std::string, std::size_t> s2 = ppl::make(length);
  ppl::segment<ppl::terminated, std::string> s1 = ppl::from(lines) | trim;
  ppl::segment<ppl::terminated, ppl::terminated> s = s1 | s2 | output;
  //]

  (void)s;
}

//[example_open_segment_type_erasure_interface
void execute_plan(ppl::plan& p)
{
  ppl::thread_pool pool{1};
  auto exec = p.run(pool);
  exec.wait();
}

ppl::segment<ppl::terminated, std::size_t> append_length(const ppl::segment<ppl::terminated, std::string>& s1)
{
  return s1 | length;
}
//]

void interface_of_type_erasured_handles()
{
  std::vector<std::string> input{"foo", "barA", "bazBB"};
  std::vector<std::size_t> output;

  auto s1 = ppl::from(input);
  auto s2 = append_length(s1);
  ppl::plan s3 = s2 | output;

  execute_plan(s3);
}

int main()
{
  use_auto();
  use_type_erasure();
  interface_of_type_erasured_handles();

  return 0;
}
