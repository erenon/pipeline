#include <string>
#include <vector>
#include <regex>
#include <functional>
#include <iterator>
#include <iostream>

#include <boost/pipeline.hpp>

std::string grep(const std::string& re, const std::string& item) {
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

std::string grep_error(const std::string& item)
{
  std::regex regex("Error.*");

  if (std::regex_match(item, regex))
  {
    return item;
  }
  else
  {
    return "no match";
  }
}

int main()
{
  std::vector<std::string> input = {
    "Error: foobar",
    "Warning: barbaz",
    "Notice: qux",
    "Error: abc"
  };

//  auto grep_error = std::bind(grep, "Error.*", std::placeholders::_1);

  std::vector<std::string> output;
  auto out_it = std::back_inserter(output);

  (boost::pipeline::from(input) | grep_error).run(out_it);

  for (auto& out_item : output)
  {
    std::cout << out_item << std::endl;
  }
}
