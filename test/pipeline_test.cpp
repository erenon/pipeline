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

#define BOOST_TEST_MODULE pipeline
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

BOOST_AUTO_TEST_CASE(From)
{

}

BOOST_AUTO_TEST_CASE(Make)
{

}

void consume1(int) {}
void consume2(queue_front<int>& qf) { int i; qf.try_pop(i); }
int  consume3(int) { return 0; }
int  consume4(queue_front<int>& qf) { int i; qf.try_pop(i); return 0; }

BOOST_AUTO_TEST_CASE(To)
{
  std::vector<int> input{0, 1, 2, 3};

  thread_pool pool(1);

  auto exec1 = (from(input) | consume1).run(pool);
  auto exec2 = (from(input) | consume2).run(pool);
  auto exec3 = (from(input) | to(consume3)).run(pool);
  auto exec4 = (from(input) | to(consume4)).run(pool);

  exec1.wait();
  exec2.wait();
  exec3.wait();
  exec4.wait();
}

BOOST_AUTO_TEST_CASE(ToFunction)
{
  auto f_consume1 = std::function<void(int)>(consume1);
  auto f_consume2 = std::function<void(queue_front<int>&)>(consume2);
  auto f_consume3 = std::function<int(int)>(consume3);
  auto f_consume4 = std::function<int(queue_front<int>&)>(consume4);

  std::vector<int> input{0, 1, 2, 3};

  thread_pool pool(1);

  auto exec1 = (from(input) | f_consume1).run(pool);
  auto exec2 = (from(input) | f_consume2).run(pool);
  auto exec3 = (from(input) | to(f_consume3)).run(pool);
  auto exec4 = (from(input) | to(f_consume4)).run(pool);

  exec1.wait();
  exec2.wait();
  exec3.wait();
  exec4.wait();
}
