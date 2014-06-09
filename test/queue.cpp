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

#include <boost/pipeline/queue.hpp>

#define BOOST_TEST_MODULE Queue
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

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
