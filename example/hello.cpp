#include <string>
#include <vector>
#include <regex>
#include <functional>
#include <iterator>
#include <iostream>
#include <deque>

#include <boost/algorithm/string/trim.hpp>

#include <boost/pipeline.hpp>

std::string grep(
  const std::string& re,
  const std::string& item,
  boost::pipeline::queue<std::string>& output
)
{
  std::regex regex(re);

  if (std::regex_match(item, regex))
  {
    output.push_back(item);
  }

  return item;
}

std::string trim(const std::string& item)
{
  return boost::algorithm::trim_copy(item);
}

int main()
{
  using namespace std::placeholders; // _1 and _2

  std::vector<std::string> input = {
    "Error: foobar",
    " Warning: barbaz",
    "Notice: qux",
    "\tError: abc"
  };

  auto grep_error = std::bind(grep, "Error.*", _1, _2);

  // boost::pipeline::from(input.begin(), input.end()) also works

  auto output =
  (boost::pipeline::from(input)
    | trim
    | grep_error
    | [] (const std::string& item) { return "->" + item; }
  ).run();

  for (auto& out_item : output)
  {
    std::cout << out_item << std::endl;
  }
}
