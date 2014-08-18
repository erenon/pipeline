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

#ifndef BOOST_PIPELINE_TYPE_ERASURE_HPP
#define BOOST_PIPELINE_TYPE_ERASURE_HPP

#include <memory>

#include <boost/pipeline/threading.hpp>
#include <boost/pipeline/execution.hpp>
#include <boost/pipeline/queue.hpp>
#include <boost/pipeline/detail/segment_concept.hpp>
#include <boost/pipeline/detail/open_segment.hpp>
#include <boost/pipeline/detail/closed_segment.hpp>

namespace boost {
namespace pipeline {

typedef void terminated;

namespace detail {

//
// Utilities
//

struct unknown_type {};

template <typename Input>
class upstream_proxy
{
public:
  typedef Input root_type;
  typedef Input value_type;

  void connect_to(runnable_concept<Input>& parent)
  {
    _parent = &parent;
  }

  template <typename... Args>
  void run(Args&&... args)
  {
    _parent->run(std::forward<Args>(args)...);
  }

private:
  runnable_concept<Input>* _parent = nullptr;
};

template <typename Segment>
inline void connect_to(Segment& segment, runnable_concept<typename Segment::root_type>& parent)
{
  segment.connect_to(parent);
}

template <typename Input, typename Output>
inline void connect_to(
  segment<Input, Output>& handle,
  runnable_concept<typename segment<Input, Output>::root_type>& parent
); // implementation far below

//
// connected_segment<> and specializations
//

template <typename Input, typename Middle, typename Output>
class connected_segment
  : public segment_concept<Input, Output>
{
  connected_segment(const connected_segment<Input, Middle, Output>& rhs)
    :_parent(rhs._parent->clone()),
     _impl(rhs._impl->clone())
  {
    _impl->connect_to(*_parent);
  }

public:
  typedef Input  root_type;
  typedef Output value_type;

  connected_segment(
    const segment_concept<Input, Middle>& parent,
    const segment_concept<Middle, Output>& impl
  )
    :_parent(parent.clone()),
     _impl(impl.clone())
  {
    _impl->connect_to(*_parent);
  }

  std::unique_ptr<segment_concept<Input, Output>> clone() const
  {
    return std::unique_ptr<segment_concept<Input, Output>>(
      new connected_segment<Input, Middle, Output>(*this)
    );
  }

  void run(thread_pool& pool, const queue_back<Output>& target)
  {
    _impl->run(pool, target);
  }

private:
  std::unique_ptr<segment_concept<Input, Middle>> _parent;
  std::unique_ptr<segment_concept<Middle, Output>> _impl;
};

template <typename Input, typename Middle>
class connected_segment<Input, Middle, terminated>
  : public segment_concept<Input, terminated>
{
  connected_segment(const connected_segment<Input, Middle, terminated>& rhs)
    :_parent(rhs._parent->clone()),
     _impl(rhs._impl->clone())
  {
    _impl->connect_to(*_parent);
  }

public:
  typedef Input      root_type;
  typedef terminated value_type;

  connected_segment(
    const segment_concept<Input, Middle>& parent,
    const segment_concept<Middle, terminated>& impl
  )
    :_parent(parent.clone()),
     _impl(impl.clone())
  {
    _impl->connect_to(*_parent);
  }

  std::unique_ptr<segment_concept<Input, terminated>> clone() const
  {
    return std::unique_ptr<segment_concept<Input, terminated>>(
      new connected_segment<Input, Middle, terminated>(*this)
    );
  }

  execution run(thread_pool& pool)
  {
    return _impl->run(pool);
  }

private:
  std::unique_ptr<segment_concept<Input, Middle>> _parent;
  std::unique_ptr<segment_concept<Middle, terminated>> _impl;
};

} // namespace detail

//
// segment<I,O> and specializations
//

template <typename Input, typename Output>
class segment
{
  template <typename, typename>
  friend class segment;

  typedef detail::upstream_proxy<Input> proxy;

public:
  typedef Input  root_type;
  typedef Output value_type;

  segment(const segment<Input, Output>& rhs)
    :_impl(rhs._impl->clone())
  {}

  segment(const detail::segment_concept<Input, Output>& impl)
    :_impl(impl.clone())
  {}

  template <typename... Trafos>
  segment(const detail::open_segment<Trafos...>& impl)
    :_impl(impl.connect_to(proxy()).clone())
  {}

  template <typename NewOutput>
  segment<Input, NewOutput>
  operator|(const segment<Output, NewOutput>& rhs) const
  {
    return detail::connected_segment<Input, Output, NewOutput>(
      *_impl, *rhs._impl
    );
  }

  void run(thread_pool& pool, const queue_back<Output>& target)
  {
    _impl->run(pool, target);
  }

private:
  friend void detail::connect_to<>(
    segment<Input, Output>&,
    detail::runnable_concept<Input>&
  ); // don't move this above root_type typedef

  std::unique_ptr<detail::segment_concept<Input, Output>> _impl;
};

template <typename Output>
class segment<terminated, Output>
{
  template <typename, typename>
  friend class segment;

public:
  typedef terminated root_type;
  typedef Output     value_type;

  segment(const segment<terminated, Output>& rhs)
    :_impl(rhs._impl->clone())
  {}

  segment(const detail::segment_concept<terminated, Output>& impl)
    :_impl(impl.clone())
  {}

  template <typename NewOutput>
  segment<terminated, NewOutput>
  operator|(const segment<Output, NewOutput>& rhs) const
  {
    return detail::connected_segment<terminated, Output, NewOutput>(
      *_impl, *rhs._impl
    );
  }

  void run(thread_pool& pool, const queue_back<Output>& target)
  {
    _impl->run(pool, target);
  }

private:
  std::unique_ptr<detail::segment_concept<terminated, Output>> _impl;
};

template <typename Input>
class segment<Input, terminated>
{
  template <typename, typename>
  friend class segment;

  typedef detail::upstream_proxy<Input> proxy;

public:
  segment(const segment<Input, terminated>& rhs)
    :_impl(rhs._impl->clone())
  {}

  segment(const detail::segment_concept<Input, terminated>& impl)
    :_impl(impl.clone())
  {}

  template <typename... Trafos>
  segment(const detail::open_segment<Trafos...>& impl)
    :_impl(impl.connect_to(proxy()).clone())
  {}

  template <typename Trafo>
  segment(const detail::closed_segment<Trafo>& impl)
    :_impl((proxy() | impl).clone())
  {}


  execution run(thread_pool& pool)
  {
    return _impl->run(pool);
  }

private:
  std::unique_ptr<detail::segment_concept<Input, terminated>> _impl;
};

template <>
class segment<terminated, terminated>
{
  template <typename, typename>
  friend class segment;

public:
  segment(const segment<terminated, terminated>& rhs)
    :_impl(rhs._impl->clone())
  {}

  segment(const detail::segment_concept<terminated, terminated>& impl)
    :_impl(impl.clone())
  {}

  execution run(thread_pool& pool)
  {
    return _impl->run(pool);
  }

private:
  std::unique_ptr<detail::segment_concept<terminated, terminated>> _impl;
};

using plan = segment<terminated, terminated>;

namespace detail {

//
// is_connectable_segment predicate specializations
//

template <typename>
struct is_connectable_segment;

template <typename O>
struct is_connectable_segment<segment<terminated, O>> : public std::true_type {};

template <typename I, typename O>
struct is_connectable_segment<segment<I, O>> : public std::true_type {};

template <typename I>
struct is_connectable_segment<upstream_proxy<I>> : public std::true_type {};

//
// Utilities (part 2)
//

template <typename Input, typename Output>
inline void connect_to(
  segment<Input, Output>& handle,
  runnable_concept<typename segment<Input, Output>::root_type>& parent
)
{
  handle._impl->connect_to(parent);
}

} // namespace detail

} // namespace pipeline
} // namespace boost

#endif // BOOST_PIPELINE_TYPE_ERASURE_HPP
