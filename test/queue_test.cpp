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

#include <ostream>

#include <boost/pipeline/queue.hpp>

#define BOOST_TEST_MODULE Queue
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

namespace std {

std::ostream& operator<<(std::ostream& out, const queue_op_status& st)
{

  switch (st)
  {
  case queue_op_status::SUCCESS:
    out << "SUCCESS";
    break;
  case queue_op_status::FULL:
      out << "FULL";
      break;
  case queue_op_status::EMPTY:
      out << "EMPTY";
      break;
  case queue_op_status::CLOSED:
      out << "CLOSED";
      break;
  }
  return out;
}

} // namespace std

BOOST_AUTO_TEST_CASE(InterfaceBasics)
{
  queue<int> q;

  int v = 0;

  queue_op_status empty = q.try_pop(v);
  BOOST_CHECK_EQUAL(empty, queue_op_status::EMPTY);

  queue_op_status succ = q.try_push(1);
  BOOST_CHECK_EQUAL(succ, queue_op_status::SUCCESS);

  succ = q.try_pop(v);
  BOOST_CHECK_EQUAL(succ, queue_op_status::SUCCESS);
  BOOST_CHECK_EQUAL(v, 1);
}

BOOST_AUTO_TEST_CASE(Close)
{
  queue<int> q;
  BOOST_CHECK(q.is_closed() == false);

  q.close();

  BOOST_CHECK(q.is_closed());
}

BOOST_AUTO_TEST_CASE(FrontPop)
{
  queue<int> q;

  q.try_push(1);
  int front = q.front();
  BOOST_CHECK_EQUAL(front, 1);

  q.try_push(2);
  front = q.front();
  BOOST_CHECK_EQUAL(front, 1);

  q.try_pop();
  front = q.front();
  BOOST_CHECK_EQUAL(front, 2);
  q.try_pop();

  queue_op_status empty = q.try_pop();
  BOOST_CHECK_EQUAL(empty, queue_op_status::EMPTY);
}

BOOST_AUTO_TEST_CASE(EmptyFull)
{
  queue<int> q;
  BOOST_CHECK(q.is_empty());
  BOOST_CHECK( ! q.is_full());

  while(queue_op_status::SUCCESS == q.try_push(1))
    /* nop */;

  BOOST_CHECK( ! q.is_empty());
  BOOST_CHECK(q.is_full());

  while(queue_op_status::SUCCESS == q.try_pop())
    /* nop */;

  BOOST_CHECK(q.is_empty());
  BOOST_CHECK( ! q.is_full());
}

BOOST_AUTO_TEST_CASE(ReadWriteAvailable)
{
  queue<int> q;
  size_t r = 0;
  size_t w = 0;

  r = q.read_available();
  w = q.write_available();

  const size_t sum = r + w;

  q.try_push(1);
  q.try_push(1);
  q.try_push(1);

  r = q.read_available();
  w = q.write_available();

  BOOST_CHECK_EQUAL(sum, r + w);

  q.try_pop();
  q.try_pop();

  r = q.read_available();
  w = q.write_available();

  BOOST_CHECK_EQUAL(sum, r + w);
}
