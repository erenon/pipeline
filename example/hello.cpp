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
#include <regex>
#include <functional>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>

#include <boost/pipeline.hpp>

void grep(
  const std::string& re,
  boost::pipeline::queue_front<std::string>& upstream,
  boost::pipeline::queue_back<std::string>& downstream
)
{
  std::regex regex(re);

  std::string input;
  while (upstream.wait_pull(input))
  {
    if (std::regex_match(input, regex))
    {
      downstream.push(input);
    }
  }
}

std::string trim(const std::string& input)
{
  return boost::algorithm::trim_copy(input);
}

int main()
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  std::vector<std::string> input = {
    "Error: foobar",
    " Warning: barbaz",
    "Notice: qux",
    "\tError: abc"
  };

  //[ example_hello_body
  auto grep_error = std::bind(grep, "Error.*", _1, _2);

  boost::pipeline::thread_pool pool;
  std::vector<std::string> output;

  auto execution =
  (boost::pipeline::from(input)
    | trim
    | grep_error
    | [] (const std::string& input) { return "-> " + input; }
    | output
  ).run(pool);
  //]

  execution.wait();

  for (auto& out_item : output)
  {
    std::cout << out_item << std::endl;
  }
}
