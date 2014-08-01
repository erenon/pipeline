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

#include <boost/pipeline.hpp>
namespace ppl = boost::pipeline;

int id(int input)
{
  return input;
}

int mul(int multiplier, int input)
{
  return multiplier * input;
}

void take_twice(int input, ppl::queue_back<int> downstream)
{
  downstream.push(input);
  downstream.push(input);
}

void sum_diff_prod(
  ppl::queue_front<int>& upstream,
  ppl::queue_back<int>& downstream
)
{
  int a;
  int b;

  if (upstream.wait_pull(a) && upstream.wait_pull(b))
  {
    downstream.push(a+b);
    downstream.push(a-b);
    downstream.push(a*b);
  }

  // if the input size is odd, the last one will be dropped
}

int aggregate(
  int accumulator,
  std::function<int(int,int)> aggregator,
  ppl::queue_front<int>& upstream
)
{
  int input;
  while (upstream.wait_pull(input))
  {
    accumulator = aggregator(input, accumulator);
  }

  return accumulator;
}

void show(int output)
{
  std::cout << "The answer: " << output << std::endl;
}

int main()
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<int> output;

  auto mul_three = std::bind(mul, 3, _1);
  auto sum_all = std::bind(
    aggregate,
    0,
    [](int i, int a) { return a + i; },
    _1
  );

  ppl::thread_pool pool;
  auto exec =
    (ppl::from(input)
      | id | mul_three | take_twice
      | sum_diff_prod | sum_all | show
    ).run(pool);

  exec.wait();

  return 0;
}

