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

#include <memory>

#include <boost/pipeline/queue.hpp>

#define BOOST_TEST_MODULE Queue
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

BOOST_AUTO_TEST_CASE(InterfaceBasics)
{
  queue<int> q;
  queue_front<int> qf(q);
  queue_back<int>  qb(q);

  BOOST_CHECK(qf.is_closed() == false);

  qb.push(1);
  qb.push(2);
  qb.push(3);

  int input = 0;

  qf.wait_pull(input);
  BOOST_CHECK_EQUAL(input, 1);

  qf.wait_pull(input);
  BOOST_CHECK_EQUAL(input, 2);

  qb.close();

  qf.wait_pull(input);
  BOOST_CHECK_EQUAL(input, 3);

  BOOST_CHECK(qf.is_closed());
}

struct movable_only
{
  movable_only(int v) : _value(v) {}
  movable_only(movable_only&&) = default;
  movable_only(const movable_only&) = delete;
  void operator=(const movable_only&) = delete;
  movable_only& operator=(movable_only&&) = default;

  bool operator==(const movable_only& rhs)
  {
    return _value == rhs._value;
  }

  int _value;
};

BOOST_AUTO_TEST_CASE(MovableT)
{
  queue<movable_only> q;
  queue_front<movable_only> qf(q);
  queue_back<movable_only>  qb(q);

  movable_only item(1);

  qb.push(std::move(item));

  movable_only expected_ret(1);
  movable_only ret(0);
  qf.wait_pull(ret);

  BOOST_CHECK(ret == expected_ret);
}
