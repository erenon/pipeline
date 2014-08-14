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
#include <sstream>
#include <iostream>
#include <functional>

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/pipeline.hpp>
namespace ppl = boost::pipeline;

struct request
{
  bool is_priority;
  std::string body;
};

struct parsed_request
{
  boost::property_tree::ptree parameters;
};

struct response
{
  std::string body;
};

void generate_requests(ppl::queue_back<request>& downstream)
{
  for (int i = 0; i < 1000; ++i)
  {
    request output;
    output.is_priority = (i % 10 == 0);

    std::string id = boost::lexical_cast<std::string>(i);
    output.body = "{ \"id\": " + id +" }";

    downstream.push(std::move(output));
  }
}

//[example_split_splitter
void split_by_priority(
  ppl::queue_back<request> priority,
  ppl::queue_back<request> non_priority,
  const request& input
)
{
  if (input.is_priority)
  {
    priority.push(input);
  }
  else
  {
    non_priority.push(input);
  }
}
//]

void parse_request(const request& input, ppl::queue_back<parsed_request>& downstream)
{
  parsed_request output;

  std::istringstream json_input(input.body);

  try
  {
    boost::property_tree::read_json(json_input, output.parameters);
    downstream.push(std::move(output));
  }
  catch (const boost::property_tree::json_parser_error& ex)
  {
    // log error
  }
}

response request_id(const parsed_request& input)
{
  response output;

  if (input.parameters.count("id"))
  {
    output.body = "Priority request id: " + input.parameters.get<std::string>("id");
  }
  else
  {
    output.body = "Unknown request id";
  }

  return output;
}

void to_stdout(const response& input)
{
  std::cout << input.body << std::endl;
}

void process_later(const response&) { /* do nothing */ }

int main()
{
  using std::placeholders::_1;

  //[example_split_invocation
  ppl::queue<request> priority_queue;
  ppl::queue<request> non_priority_queue;

  auto split = std::bind(
    split_by_priority,
    std::ref(priority_queue),
    std::ref(non_priority_queue),
    _1
  );

  auto reader = ppl::from(generate_requests) | split;

  auto priority_processor = priority_queue | parse_request | request_id | to_stdout;
  auto processor      = non_priority_queue | parse_request | request_id | process_later;
  //]

  ppl::thread_pool pool{8};

  auto exec1 = reader.run(pool);
  auto exec2 = priority_processor.run(pool);
  auto exec3 = processor.run(pool);

  exec1.wait();

  priority_queue.close();
  non_priority_queue.close();

  exec2.wait();
  exec3.wait();

  return 0;
}
