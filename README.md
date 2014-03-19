Boost.Pipeline
==============

C++ Pipeline implementation based on [n3534][0]. 
This preview is a supplement of a GSoC 2014 proposal to Boost.
The project also have an [example documentation][1], generated using quickbook.
The [submitted proposal][2] is publicly available to review and comment.

Example
-------

Using pipelines it's easy do define isolated transformations which can run parallel.

The following example uses this library preview and its full version can be found in the `example/` directory.

    auto grep_error = std::bind(grep, "Error.*", std::placeholders::_1);

    (boost::pipeline::from(input)
      | trim
      | grep_error
      | [] (const std::string& item) { return "->" + item; }
    ).run(out_it);
  
  
How to Build
------------

This library is header only, there is no need to build anything specific to use it.
However, there are examples and tests to be build:

    git clone https://github.com/erenon/pipeline.git ./pipeline/
    # build examples
    # artifacts will be in: /var/tmp/pipeline/
    cd pipeline/build
    bjam toolset=gcc # or clang
    # build and run tests
    cd ../test
    bjam toolset=gcc # or clang

[0]: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3534.html
[1]: http://erenon.hu/pipeline/
[2]: https://gist.github.com/erenon/9537711
