#include <string>
#include <vector>
#include <regex>
#include <functional>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>

#include <boost/pipeline.hpp>

std::string grep(
  const std::string& re,
  const std::string& item,
  boost::pipeline::queue_back<std::string>& output
)
{
  std::regex regex(re);

  if (std::regex_match(item, regex))
  {
    output.push(item);
  }

  return item;
}

std::string trim(const std::string& item)
{
  return boost::algorithm::trim_copy(item);
}

int main()
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  std::vector<std::string> input = {
    "Error: foobar",
    " Warning: barbaz",
    "Notice: qux",
    "\tError: abc"
  };

  auto grep_error = std::bind(grep, "Error.*", _1, _2);

  boost::pipeline::thread_pool pool;

  std::vector<std::string> output;

  auto execution =
  (boost::pipeline::from(input)
    | trim
    | grep_error
    | [] (const std::string& item) { return "->" + item; }
    | output
  ).run(pool);

  execution.wait();

  for (auto& out_item : output)
  {
    std::cout << out_item << std::endl;
  }
}
