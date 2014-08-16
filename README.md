Boost.Pipeline
==============

C++ Pipeline implementation based on [N3534][0].
Development of this library is founded by Google through the GSoC 2014 program.
Please refer to the [**documentation**][1] for more information.

Example
-------

Using pipelines it's easy do define isolated transformations which can run parallel.

The following example uses this library preview and its full version can be found in the `example/` directory.

    auto grep_error = std::bind(grep, "Error.*", _1, _2);

    (boost::pipeline::from(input)
      | trim
      | grep_error
      | [] (const std::string& item) { return "-> " + item; }
      | output
    ).run(pool);
  
  
Feedback
--------

Altough the library is not stable and under development, feedback is welcome.

[0]: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3534.html
[1]: http://erenon.hu/pipeline/
