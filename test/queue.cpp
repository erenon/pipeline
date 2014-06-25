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

std::ostream& operator<<(std::ostream& out, const queue<int>::op_status& st)
{
  using status = queue<int>::op_status;

  switch (st)
  {
  case status::SUCCESS:
    out << "SUCCESS";
    break;
  case status::FAILURE:
      out << "FAILURE";
      break;
  case status::CLOSED:
      out << "CLOSED";
      break;
  }
  return out;
}

} // namespace std

BOOST_AUTO_TEST_CASE(InterfaceBasics)
{
  using status = queue<int>::op_status;
  queue<int> q;

  int v = 0;

  status fail = q.try_pop(v);
  BOOST_CHECK_EQUAL(fail, status::FAILURE);

  status succ = q.try_push(1);
  BOOST_CHECK_EQUAL(succ, status::SUCCESS);

  succ = q.try_pop(v);
  BOOST_CHECK_EQUAL(succ, status::SUCCESS);
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
  using status = queue<int>::op_status;
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

  status failure = q.try_pop();
  BOOST_CHECK_EQUAL(failure, status::FAILURE);
}

BOOST_AUTO_TEST_CASE(EmptyFull)
{
  using status = queue<int>::op_status;

  queue<int> q;
  BOOST_CHECK(q.empty());
  BOOST_CHECK( ! q.full());

  while(status::SUCCESS == q.try_push(1))
    /* nop */;

  BOOST_CHECK( ! q.empty());
  BOOST_CHECK(q.full());

  while(status::SUCCESS == q.try_pop())
    /* nop */;

  BOOST_CHECK(q.empty());
  BOOST_CHECK( ! q.full());
}
