#include <string>
#include <vector>
#include <regex>
#include <functional>
#include <iterator>
#include <iostream>
#include <deque>

#include <boost/algorithm/string/trim.hpp>

#include <boost/pipeline.hpp>


std::string grep(const std::string& re, const std::string& item)
{
  std::regex regex(re);

  if (std::regex_match(item, regex))
  {
    return item;
  }
  else
  {
    return "no match";
  }
}

//void grep(
//  const std::string& re,
//  const std::string& item,
//  std::back_insert_iterator<std::deque<std::string>>& it
//)
//{
//  std::regex regex(re);
//
//  if (std::regex_match(item, regex))
//  {
//    *it = item;
//    ++it;
//  }
//}

std::string trim(const std::string& item)
{
  return boost::algorithm::trim_copy(item);
}

int main()
{
  std::vector<std::string> input = {
    "Error: foobar",
    " Warning: barbaz",
    "Notice: qux",
    "\tError: abc"
  };

  auto grep_error = std::bind(grep, "Error.*", std::placeholders::_1);

  std::vector<std::string> output;
  auto out_it = std::back_inserter(output);

  // boost::pipeline::from(input.begin(), input.end()) also works

  (boost::pipeline::from(input)
    | trim
    | grep_error
    | [] (const std::string& item) { return "->" + item; }
  ).run(out_it);

  for (auto& out_item : output)
  {
    std::cout << out_item << std::endl;
  }
}
