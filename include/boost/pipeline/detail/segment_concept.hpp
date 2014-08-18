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

#ifndef BOOST_PIPELINE_DETAIL_SEGMENT_CONCEPT_HPP
#define BOOST_PIPELINE_DETAIL_SEGMENT_CONCEPT_HPP

#include <boost/pipeline/threading.hpp>
#include <boost/pipeline/queue.hpp>

namespace boost {
namespace pipeline {

typedef void terminated;

template <typename Input, typename Output>
class segment;

namespace detail {

//
// Concepts
//

template <typename Output>
class runnable_concept
{
public:
  virtual ~runnable_concept() {}
  virtual void run(thread_pool&, const queue_back<Output>&) = 0;
};

template <>
class runnable_concept<terminated>
{
public:
  virtual ~runnable_concept() {}
  virtual execution run(thread_pool&) = 0;
};

template <typename Input, typename Output>
class segment_concept
  : public runnable_concept<Output>
{
public:
  typedef Input root_type;
  typedef Output value_type;

  virtual ~segment_concept() {}
  virtual std::unique_ptr<segment_concept<Input, Output>> clone() const = 0;
  virtual void connect_to(runnable_concept<Input>&) = 0;
};

template <typename Output>
class segment_concept<terminated, Output>
  : public runnable_concept<Output>
{
public:
  typedef terminated root_type;
  typedef Output value_type;

  virtual ~segment_concept() {}
  virtual std::unique_ptr<segment_concept<terminated, Output>> clone() const = 0;
};

template <>
class segment_concept<terminated, terminated>
  : public runnable_concept<terminated>
{
public:
  typedef terminated root_type;
  typedef terminated value_type;

  virtual ~segment_concept() {}
  virtual std::unique_ptr<segment_concept<terminated, terminated>> clone() const = 0;
};

} // namespace detail

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_DETAIL_SEGMENT_CONCEPT_HPP
