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
#include <vector>
#include <iostream>
#include <functional>

#include <boost/pipeline.hpp>
namespace ppl = boost::pipeline;

struct department
{
  int id;
  std::string name;
};

struct person
{
  std::string name;
  int department_id;
};

struct relation
{
  std::string department_name;
  std::string person_name;
};

//[example_join_joiner
void join(
  ppl::queue_front<department> deps,
  ppl::queue_front<person> perss,
  ppl::queue_back<relation> downstream
)
{
  department dep;
  person pers{"", -1};

  while (deps.wait_pull(dep))
  {
    if (dep.id == pers.department_id)
    {
      relation rel{dep.name, pers.name};
      downstream.push(std::move(rel));
    }

    while (perss.wait_pull(pers))
    {
      if (dep.id == pers.department_id)
      {
        relation rel{dep.name, pers.name};
        downstream.push(std::move(rel));
      }
      else
      {
        break;
      }
    }
  }
}
//]

void to_stdout(const relation& input)
{
  std::cout << input.person_name << " works at "
            << input.department_name << std::endl;
}

void init_departments(ppl::queue<department>& queue)
{
  std::vector<department> deps = {
    {0, "Board"},
    {1, "IT"},
    {2, "HR"},
    {3, "Finance"},
    {4, "Legal"}
  };

  for (const auto& dep : deps)
  {
    queue.push(dep);
  }
}

void init_persons(ppl::queue<person>& queue)
{
  std::vector<person> perss = {
    {"Niamh Devin", 0},
    {"Jayna Vera", 0},
    {"Jenci Anneka", 1},
    {"Hector Inga", 1},
    {"Agathe Jarod", 1},
    {"Tivadar Carolin", 2},
    {"Amalia Forest", 2},
    {"Nita Emmerich", 2},
    {"Elisabeth Keavy", 4},
    {"Loraine Rian", 4}
  };

  for (const auto& pers : perss)
  {
    queue.push(pers);
  }
}

int main()
{
  using std::placeholders::_1;

  ppl::queue<department> departments;
  ppl::queue<person> persons;

  init_departments(departments);
  init_persons(persons);

  departments.close();
  persons.close();

  //[example_join_invocation
  auto relations = std::bind(join, std::ref(departments), std::ref(persons), _1);
  auto plan = ppl::from<relation>(relations) | to_stdout;
  //]

  ppl::thread_pool pool{2};
  auto exec = plan.run(pool);

  exec.wait();

  return 0;
}
