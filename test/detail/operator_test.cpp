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

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE OperatorTest
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

/*
 * Available components
 *
 * Producers (Stage 1):
 *
 * from(input)
 * input_queue
 * from(generator)
 *
 * Transformations (Stage 2/3):
 *
 * ab_oo / ba_oo
 * ab_on / ba_on
 * ab_nm / ba_nm
 * ab_no / ba_no
 *
 * Consumers (Stage 4):
 *
 * output
 * output_queue
 * consume
 * to(consume)
 *
 * Combinators:
 * make -- Stage 2-4
 * segment<I,O> -- Stage 1-4
 */

// Value types

struct Argon
{
  int value;
};

struct Barium
{
  int value;
};

// Producers

void init_vector(std::vector<Argon>& input)
{
  for (int i = 0; i < 100; ++i)
  {
    Argon a{i};
    input.push_back(std::move(a));
  }
}

void init_queue(queue<Argon>& input_queue)
{
  for (int i = 0; i < 100; ++i)
  {
    Argon a{i};
    input_queue.push(std::move(a));
  }

  input_queue.close();
}

void generator(queue_back<Argon>& downstream)
{
  for (int i = 0; i < 100; ++i)
  {
    Argon a{i};
    downstream.push(std::move(a));
  }
}

// Transformations

Barium ab_oo(const Argon& input)
{
  return Barium{input.value};
}

void ab_on(const Argon& input, queue_back<Barium>& downstream)
{
  downstream.push(std::move(Barium{input.value}));
}

void ab_nm(queue_front<Argon>& upstream, queue_back<Barium>& downstream)
{
  Argon input;
  if (upstream.wait_pull(input))
  {
    downstream.push(std::move(Barium{input.value}));
  }
}

Barium ab_no(queue_front<Argon>& upstream)
{
  Argon input;
  if (upstream.wait_pull(input))
  {
    return Barium{input.value};
  }

  return Barium{-1};
}

Argon ba_oo(const Barium& input)
{
  return Argon{input.value};
}

void ba_on(const Barium& input, queue_back<Argon>& downstream)
{
  downstream.push(std::move(Argon{input.value}));
}

void ba_nm(queue_front<Barium>& upstream, queue_back<Argon>& downstream)
{
  Barium input;
  if (upstream.wait_pull(input))
  {
    downstream.push(std::move(Argon{input.value}));
  }
}

Argon ba_no(queue_front<Barium>& upstream)
{
  Barium input;
  if (upstream.wait_pull(input))
  {
    return Argon{input.value};
  }

  return Argon{-1};
}

// Consumers

int g_value_sum;

void consume(const Argon& input)
{
  g_value_sum += input.value;
}

// Verifiers

void verify_vector(const std::vector<Argon>& output)
{
  BOOST_CHECK_EQUAL(output.size(), 100u);

  for (int i = 0; i < 100; ++i)
  {
    BOOST_CHECK_EQUAL(output[i].value, i);
  }
}

void verify_queue(queue<Argon>& output)
{
  BOOST_CHECK_EQUAL(output.size(), 100u);

  for (int i = 0; i < 100; ++i)
  {
    Argon out;
    output.wait_pull(out);
    BOOST_CHECK_EQUAL(out.value, i);
  }
}

void verify_consumed()
{
  BOOST_CHECK_EQUAL(g_value_sum, 4950);
}

// GENERATED CODE -- edit operator_test_generator.cpp instead

BOOST_AUTO_TEST_CASE(Container_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_OO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_oo | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_OO_OO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_oo | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_OO_OO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_OO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_ON_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_oo | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_OO_ON_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_oo | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_OO_ON_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_ON_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_NM_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_oo | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_OO_NM_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_oo | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_OO_NM_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_NM_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_NO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_oo | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_OO_NO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_oo | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_OO_NO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_OO_NO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_oo | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_OO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_on | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_ON_OO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_on | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_ON_OO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_OO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_ON_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_on | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_ON_ON_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_on | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_ON_ON_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_ON_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_NM_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_on | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_ON_NM_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_on | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_ON_NM_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_NM_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_NO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_on | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_ON_NO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_on | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_ON_NO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_ON_NO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_on | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_OO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_nm | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NM_OO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_nm | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NM_OO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_OO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_ON_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_nm | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NM_ON_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_nm | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NM_ON_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_ON_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_NM_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_nm | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NM_NM_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_nm | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NM_NM_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_NM_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_NO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_nm | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NM_NO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_nm | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NM_NO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NM_NO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_nm | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_OO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_no | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NO_OO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_no | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NO_OO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_OO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_ON_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_no | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NO_ON_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_no | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NO_ON_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_ON_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_NM_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_no | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NO_NM_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_no | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NO_NM_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_NM_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_NO_Container)
{
  std::vector<Argon> input;
  init_vector(input);
  std::vector<Argon> output;
  auto plan = from(input) | ab_no | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Container_NO_NO_Queue)
{
  std::vector<Argon> input;
  init_vector(input);
  queue<Argon> output_queue;
  auto plan = from(input) | ab_no | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Container_NO_NO_Consumer)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Container_NO_NO_To)
{
  std::vector<Argon> input;
  init_vector(input);
  g_value_sum = 0;
  auto plan = from(input) | ab_no | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_OO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_oo | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_OO_OO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_oo | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_OO_OO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_OO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_ON_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_oo | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_OO_ON_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_oo | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_OO_ON_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_ON_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_NM_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_oo | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_OO_NM_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_oo | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_OO_NM_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_NM_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_NO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_oo | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_OO_NO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_oo | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_OO_NO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_OO_NO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_oo | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_OO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_on | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_ON_OO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_on | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_ON_OO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_OO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_ON_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_on | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_ON_ON_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_on | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_ON_ON_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_ON_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_NM_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_on | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_ON_NM_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_on | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_ON_NM_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_NM_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_NO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_on | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_ON_NO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_on | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_ON_NO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_ON_NO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_on | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_OO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_nm | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NM_OO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_nm | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NM_OO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_OO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_ON_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_nm | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NM_ON_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_nm | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NM_ON_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_ON_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_NM_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_nm | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NM_NM_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_nm | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NM_NM_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_NM_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_NO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_nm | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NM_NO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_nm | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NM_NO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NM_NO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_nm | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_OO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_no | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NO_OO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_no | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NO_OO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_OO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_ON_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_no | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NO_ON_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_no | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NO_ON_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_ON_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_NM_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_no | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NO_NM_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_no | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NO_NM_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_NM_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_NO_Container)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  std::vector<Argon> output;
  auto plan = input_queue | ab_no | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Queue_NO_NO_Queue)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  queue<Argon> output_queue;
  auto plan = input_queue | ab_no | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Queue_NO_NO_Consumer)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Queue_NO_NO_To)
{
  queue<Argon> input_queue;
  init_queue(input_queue);
  g_value_sum = 0;
  auto plan = input_queue | ab_no | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_OO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_oo | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_OO_OO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_oo | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_OO_OO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_OO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_ON_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_oo | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_OO_ON_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_oo | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_OO_ON_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_ON_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_NM_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_oo | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_OO_NM_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_oo | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_OO_NM_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_NM_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_NO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_oo | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_OO_NO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_oo | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_OO_NO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_OO_NO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_oo | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_OO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_on | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_ON_OO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_on | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_ON_OO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_OO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_ON_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_on | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_ON_ON_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_on | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_ON_ON_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_ON_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_NM_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_on | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_ON_NM_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_on | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_ON_NM_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_NM_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_NO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_on | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_ON_NO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_on | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_ON_NO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_ON_NO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_on | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_OO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_nm | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NM_OO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_nm | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NM_OO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_OO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_ON_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_nm | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NM_ON_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_nm | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NM_ON_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_ON_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_NM_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_nm | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NM_NM_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_nm | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NM_NM_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_NM_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_NO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_nm | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NM_NO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_nm | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NM_NO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NM_NO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_nm | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_OO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_no | ba_oo | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NO_OO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_no | ba_oo | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NO_OO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_oo | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_OO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_oo | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_ON_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_no | ba_on | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NO_ON_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_no | ba_on | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NO_ON_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_on | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_ON_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_on | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_NM_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_no | ba_nm | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NO_NM_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_no | ba_nm | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NO_NM_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_nm | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_NM_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_nm | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_NO_Container)
{
  std::vector<Argon> output;
  auto plan = from(generator) | ab_no | ba_no | output;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_vector(output);
}

BOOST_AUTO_TEST_CASE(Generator_NO_NO_Queue)
{
  queue<Argon> output_queue;
  auto plan = from(generator) | ab_no | ba_no | output_queue;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_queue(output_queue);
}

BOOST_AUTO_TEST_CASE(Generator_NO_NO_Consumer)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_no | consume;

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}

BOOST_AUTO_TEST_CASE(Generator_NO_NO_To)
{
  g_value_sum = 0;
  auto plan = from(generator) | ab_no | ba_no | to(consume);

  thread_pool pool{4};
  auto exec = plan.run(pool);
  exec.wait();

  verify_consumed();
}
