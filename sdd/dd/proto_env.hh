#ifndef _SDD_DD_PROTO_ENV_HH_
#define _SDD_DD_PROTO_ENV_HH_

#include "sdd/dd/stack.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Value, typename Successor>
struct internal_proto_env
{
  using value_type      = Value;
  using successor_type  = Successor;

  using value_stack_type     = stack<value_type>;
  using successor_stack_type = stack<successor_type>;

  const unsigned int level;
  const value_stack_type values;
  const successor_stack_type successors;

  /// @brief Default constructor.
  ///
  /// Create an empty environment.
  internal_proto_env()
    : level(0), values(), successors()
  {}

  internal_proto_env(unsigned int l, value_stack_type&& v, successor_stack_type&& s)
    : level(l), values(std::move(v)), successors(std::move(s))
  {}

  bool
  operator==(const internal_proto_env& other)
  const noexcept
  {
    return level == other.level and values == other.values and successors == other.successors;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A unified proto environment.
template <typename C, typename Successor>
class proto_env
{
public:

  using value_type = typename C::Values::value_type;
  using successor_type = Successor;

  using data_type   = internal_proto_env<value_type, successor_type>;
  using unique_type = mem::ref_counted<data_type>;
  using ptr_type    = mem::ptr<unique_type>;

  using value_stack_type     = typename data_type::value_stack_type;
  using successor_stack_type = typename data_type::successor_stack_type;

private:

  ptr_type ptr_;

public:

  proto_env(const proto_env&) noexcept            = default;
  proto_env& operator=(const proto_env&) noexcept = default;
  proto_env(proto_env&&) noexcept                 = default;
  proto_env& operator=(proto_env&&) noexcept      = default;

  proto_env(unsigned int level, value_stack_type&& v, successor_stack_type&& s)
    : ptr_(mk_ptr(level, std::move(v), std::move(s)))
  {}

  proto_env(ptr_type ptr)
  noexcept
    : ptr_(ptr)
  {}

  const data_type&
  operator*()
  const noexcept
  {
    return ptr_->data();
  }

  const data_type*
  operator->()
  const noexcept
  {
    return &ptr_->data();
  }

  const ptr_type&
  ptr()
  const noexcept
  {
    return ptr_;
  }

  static
  ptr_type
  empty_ptr()
  noexcept
  {
    return global<C>().empty_proto_env;
  }

  bool
  empty()
  const noexcept
  {
    return ptr_ == empty_ptr();
  }

  unsigned int
  level()
  const noexcept
  {
    return ptr_->data().level;
  }

  const value_stack_type&
  values_stack()
  const noexcept
  {
    return ptr_->data().values;
  }

  const successor_stack_type&
  successors_stack()
  const noexcept
  {
    return ptr_->data().successors;
  }

private:

  static
  ptr_type
  mk_ptr(unsigned level, value_stack_type&& v, successor_stack_type&& s)
  {
    auto& ut = global<C>().proto_env_unique_table;
    char* addr = ut.allocate(0 /* extra bytes */);
    unique_type* u = new (addr) unique_type(level, std::move(v), std::move(s));
    return ptr_type(ut(u));
  }
}; // class proto_env;

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related proto_env
template <typename C, typename Successor>
inline
proto_env<C, Successor>
empty_proto_env()
{
  return proto_env<C, Successor>::empty_ptr();
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related proto_env
template <typename C, typename Successor>
inline
bool
operator==(const proto_env<C, Successor>& lhs, const proto_env<C, Successor>& rhs)
{
  return lhs.ptr() == rhs.ptr();
}

/// @internal
/// @related proto_env
template <typename C, typename Successor>
inline
bool
operator<(const proto_env<C, Successor>& lhs, const proto_env<C, Successor>& rhs)
{
  return lhs.ptr() < rhs.ptr();
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::dd::internal_proto_env
template <typename Value, typename Successor>
struct hash<sdd::dd::internal_proto_env<Value, Successor>>
{
  std::size_t
  operator()(const sdd::dd::internal_proto_env<Value, Successor>& c)
  const
  {
    std::size_t seed = sdd::util::hash(c.level);
    sdd::util::hash_combine(seed, c.values);
    sdd::util::hash_combine(seed, c.successors);
    return seed;
  }
};

/// @internal
/// @brief Hash specialization for sdd::dd::proto_env
template <typename C, typename Successor>
struct hash<sdd::dd::proto_env<C, Successor>>
{
  std::size_t
  operator()(const sdd::dd::proto_env<C, Successor>& c)
  const noexcept
  {
    return sdd::util::hash(c.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_PROTO_ENV_HH_
