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
#include <cassert>

//[example_tutorial_preamble
#include <boost/pipeline.hpp>
namespace ppl = boost::pipeline;
//]

int mod(int modulo, int input)
{
  return input % modulo;
}

void even_only(int input, ppl::queue_back<int> downstream)
{
  if (input % 2 == 0)
  {
    downstream.push(input);
  }
}

int add(int addition, int input)
{
  return addition + input;
}

void chaining()
{
  using std::placeholders::_1;

  auto mod_seven = std::bind(mod, 7, _1);
  auto add_two = std::bind(add, 2, _1);

  std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  auto s1 =
  //[example_tutorial_chaining
  ppl::from(input) | mod_seven | even_only | add_two;
  //]

  std::vector<int> output;
  auto s = s1 | output;

  {
    ppl::thread_pool pool;
    s.run(pool);
  }

  std::vector<int> expected_output = {4, 6, 8, 2, 4};
  assert(output == expected_output && "Incorrect output produced");
}

void running()
{
  using std::placeholders::_1;

  auto mod_seven = std::bind(mod, 7, _1);
  auto add_two = std::bind(add, 2, _1);

  std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<int> output;

  //[example_tutorial_run
  ppl::thread_pool pool{4}; // add 4 threads to the pool
  ppl::execution exec = (ppl::from(input) | mod_seven | even_only | add_two | output).run(pool);
  exec.wait(); // blocks until the pipeline is finished
  bool done = exec.is_done(); // done == true
  //]

  assert(done == true && "is_done() returned false after wait");

  std::vector<int> expected_output = {4, 6, 8, 2, 4};
  assert(output == expected_output && "Incorrect output produced");
}

int main()
{
  chaining();
  running();

  return 0;
}
