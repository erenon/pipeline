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

// This test suite mimics the structure and behaviour of the
// test of the original google implementation:
// https://code.google.com/p/google-concurrency-library/source/browse/testing/pipeline_test.cc
// It's here for demonstrational purposes

#include <deque>
#include <string>

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE GooglePipeline
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

// Code (almost) verbatim copied from google:
// Dummy User class
class User {
 public:
  User(int uid = 0) : uid_(uid) {
  }
  std::string get_name() const {
    std::stringstream o;
    o << "(User : " << uid_ << ")";
    return o.str();
  }
 private:
  int uid_;
};

std::ostream& operator<<(std::ostream& os, const User u) {
  os << u.get_name();
  return os;
}

// std::string -> UID
int find_uid(std::string val) {
  printf("find_uid for %s\n", val.c_str());
  return val.length();
}

// UID -> User
User get_user(int uid) {
  printf("get for %d\n", uid);
  return User(uid);
}

// Processes the User
/*void*/int consume_user(User input) {
  printf("Consuming user %s\n", input.get_name().c_str());
  return 1;
}

void consume_string(std::string input) {
  printf("Consuming %s\n", input.c_str());
}

std::string process_string(std::string input) {
  return input + "(processed)";
}

void print_string(std::string s) {
  printf("%s", s.c_str());
}

void repeat(int i, queue_back<int> q) {
  q.try_push(i);
  q.try_push(i);
}

int sum_two(queue_front<int> q) {
  if (q.empty())
  {
    return -1;
  }

  int i = q.front();
  q.try_pop();

  if (q.empty())
  {
    return i;
  }

  int j = q.front();
  q.try_pop();

  return i + j;
}

void produce_strings(queue_back<std::string>& queue) {
  printf("Producing strings\n");
  queue.try_push("Produced String1");
  queue.try_push("Produced String22");
  queue.try_push("Produced String333");
  queue.try_push("Produced String4444");
}
// end verbatim copy

BOOST_AUTO_TEST_CASE(ManualBuild)
{
  std::deque<int> queue{1, 2, 3, 4, 5};
  auto p1 = from(queue);

  auto p6 = make(repeat);
  auto p7 = make(sum_two);
  auto p2 = make(get_user);
  auto p3 = p1 | p6 | p7 | p2;

  auto p4 = make(consume_user); // TODO ::to is missing

  auto p = p3 | p4;

  thread_pool pool{1};

  auto exec = p.run(pool);

  // TODO queue.push and close
}

BOOST_AUTO_TEST_CASE(Example)
{
  std::deque<std::string> queue{"Foo", "Bar", "Baz", "Qux"};

  auto p1 = make(find_uid);
  auto p2 = p1 | repeat;
  // auto p3 = queue | p2 | get_user; // TODO missing container | plan
  auto p3 = from(queue) | p2 | get_user;

  std::deque<User> out;
  auto p4 = p3 | out;

  thread_pool pool{1};

  auto pex  = p4.run(pool);
  auto pex2 = (from(out) | consume_user).run(pool);

  pex.wait();

  BOOST_ASSERT(pex.is_done());
  BOOST_ASSERT(pex2.is_done());

  // TODO queue is closed
}

// TODO SimpleParallel
// TODO ParallelExample

//BOOST_AUTO_TEST_CASE(ProduceExample)
//{
//  // TODO from(functor) not supported
////  auto p5 = from(produce_strings) | find_uid | get_user | consume_user;
//
//  queue_back<std::string> queue;
//  produce_strings(queue);
//
//  auto p5 = from(queue) | find_uid | get_user | consume_user;
//
//  thread_pool pool{1};
//
//  auto pex3 = p5.run(pool);
//  pex3.wait();
//}
