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
#include <string>
#include <sstream>
#include <iostream>

#include <boost/algorithm/string/join.hpp>

struct component
{
  std::string name;
  std::string header;
  std::string connectable;
  std::string footer;
};

struct combination
{
  std::vector<std::string> names;
  std::vector<std::string> headers;
  std::vector<std::string> connectables;
  std::vector<std::string> footers;
};

combination create_combination(
  const component& a, const component& b
)
{
  combination comb {
    {a.name, b.name},
    {a.header, b.header},
    {a.connectable, b.connectable},
    {a.footer, b.footer}
  };

  return std::move(comb);
}

combination create_combination(
  const component& a, const component& b, const component& c, const component& d
)
{
  combination comb {
    {a.name, b.name, c.name, d.name},
    {a.header, b.header, c.header, d.header},
    {a.connectable, b.connectable, c.connectable, d.connectable},
    {a.footer, b.footer, c.footer, d.footer}
  };

  return std::move(comb);
}

std::string join(const std::vector<std::string>& fields, const std::string& sep)
{
  return std::move(
    boost::algorithm::join_if(fields, sep, [] (const std::string& s) { return !s.empty(); })
  );
}

void print_testcase(const combination& comb)
{
  std::stringstream t;
  t << "BOOST_AUTO_TEST_CASE(" << join(comb.names, "_") << ")" "\n"
       "{" "\n"
    << "  " << join(comb.headers, "\n  ") << "\n"

    << "  auto plan = " << join(comb.connectables, " | ") << ";" "\n\n"

    "  thread_pool pool{4};" "\n"
    "  auto exec = plan.run(pool);" "\n"
    "  exec.wait();" "\n\n"

    << "  " << join(comb.footers, "\n") << "\n"

    "}" "\n"
    ;
  std::cout << t.str() << std::endl;
}

int main()
{
  std::vector<component> producers = {
    {"Container", "std::vector<Argon> input;\n  init_vector(input);", "from(input)", ""},
    {"Queue", "queue<Argon> input_queue;\n  init_queue(input_queue);", "input_queue", ""},
    {"Generator", "", "from(generator)", ""}
  };

  std::vector<component> trafos1 = {
    {"OO", "", "ab_oo", ""},
    {"ON", "", "ab_on", ""},
    {"NM", "", "ab_nm", ""},
    {"NO", "", "ab_no", ""}
  };

  std::vector<component> trafos2 = {
    {"OO", "", "ba_oo", ""},
    {"ON", "", "ba_on", ""},
    {"NM", "", "ba_nm", ""},
    {"NO", "", "ba_no", ""}
  };

  std::vector<component> consumers = {
    {"Container", "std::vector<Argon> output;", "output", "verify_vector(output);"},
    {"Queue", "queue<Argon> output_queue;", "output_queue", "verify_queue(output_queue);"},
    {"Consumer", "g_value_sum = 0;", "consume", "verify_consumed();"},
    {"To", "g_value_sum = 0;", "to(consume)", "verify_consumed();"}
  };

  for (const auto& producer : producers)
  {
    for (const auto& consumer : consumers)
    {
      auto comb = create_combination(producer, consumer);
      print_testcase(comb);
    }
  }

  for (const auto& producer : producers)
  {
    for (const auto& trafo1 : trafos1)
    {
      for (const auto& trafo2 : trafos2)
      {
        for (const auto& consumer : consumers)
        {
          auto comb = create_combination(producer, trafo1, trafo2, consumer);
          print_testcase(comb);

          // TODO add make(), segment<I,O> connectables
        }
      }
    }
  }

  return 0;
}
