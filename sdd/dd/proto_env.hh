#ifndef _SDD_DD_PROTO_ENV_HH_
#define _SDD_DD_PROTO_ENV_HH_

//#include <type_traits>
//#include <vector>

//#include "sdd/dd/definition_fwd.hh"
//#include "sdd/dd/sparse_stack.hh"
//#include "sdd/mem/ptr.hh"
//#include "sdd/mem/ref_counted.hh"
//#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

//template <typename C>
//struct sparse_value<SDD<C>>
//{
//  using value_type = SDD<C>;
//  static value_type default_value() noexcept {return zero<C>();}
//};
//
/*------------------------------------------------------------------------------------------------*/

///// @internal
//template <typename Value, typename Successor>
//struct internal_env
//{
//  using value_type      = Value;
//  using successor_type  = Successor;
//
//  using value_stack_type     = sparse_stack<sparse_value<value_type>>;
//  using successor_stack_type = sparse_stack<sparse_value<successor_type>>;
//
//  const unsigned int level;
//  const value_stack_type values;
//  const successor_stack_type successors;
//
//  /// @brief Default constructor.
//  ///
//  /// Create an empty environment.
//  internal_env()
//    : level(0), values(), successors()
//  {
//    std::cout << "internal_env()" << std::endl;
//  }
//
//  internal_env(unsigned int l, value_stack_type&& v, successor_stack_type&& s)
//    : level(l), values(std::move(v)), successors(std::move(s))
//  {
//    std::cout << "internal_env() 2" << std::endl;
//  }
//
//  bool
//  operator==(const internal_env& other)
//  const noexcept
//  {
//    return level == other.level and values == other.values and successors == other.successors;
//  }
//};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A unified proto environment.
template <typename C>
class proto_env
{
public:

//  using value_type = typename C::Values::value_type;
//  using successor_type = SDD<C>;
//
//  using data_type   = internal_env<value_type, successor_type>;
//  using unique_type = mem::ref_counted<data_type>;
//  using ptr_type    = mem::ptr<unique_type>;

//  using value_stack_type     = typename data_type::value_stack_type;
//  using successor_stack_type = typename data_type::successor_stack_type;

private:

//  ptr_type ptr_;

public:

//  env(const env&) noexcept            = default;
//  env& operator=(const env&) noexcept = default;
//  env(env&&) noexcept                 = default;
//  env& operator=(env&&) noexcept      = default;

//  env(unsigned int level, value_stack_type&& v, successor_stack_type&& s)
//    : ptr_(mk_ptr(level, std::move(v), std::move(s)))
//  {}

//  env(ptr_type ptr)
//  noexcept
//    : ptr_(ptr)
//  {}
//
//  const data_type&
//  operator*()
//  const noexcept
//  {
//    return ptr_->data();
//  }
//
//  const data_type*
//  operator->()
//  const noexcept
//  {
//    return &ptr_->data();
//  }
//
//  const ptr_type&
//  ptr()
//  const noexcept
//  {
//    return ptr_;
//  }

//  static
//  ptr_type
//  empty_ptr()
//  noexcept
//  {
//    return global<C>().empty_env;
//  }
//
//  bool
//  empty()
//  const noexcept
//  {
//    return ptr_ == empty_ptr();
//  }
//
//  unsigned int
//  level()
//  const noexcept
//  {
//    return ptr_->data().level;
//  }
//
//  const value_stack_type&
//  value_stack()
//  const noexcept
//  {
//    return ptr_->data().values;
//  }
//
//  const successor_stack_type&
//  successor_stack()
//  const noexcept
//  {
//    return ptr_->data().successors;
//  }
//
//private:
//
//  static
//  ptr_type
//  mk_ptr(unsigned level, value_stack_type&& v, successor_stack_type&& s)
//  {
//    auto& ut = global<C>().proto_unique_table;
//    char* addr = ut.allocate(0);
//    unique_type* u =
//      new (addr) unique_type(level, std::move(v), std::move(s));
//    return ptr_type(ut(u));
//  }
}; // class env;

/*------------------------------------------------------------------------------------------------*/

///// @internal
///// @related env
//template <typename C>
//inline
//env<C>
//empty_env()
//{
//  return env<C>(env<C>::empty_ptr());
//}

/*------------------------------------------------------------------------------------------------*/

///// @internal
//template <typename C>
//inline
//bool
//operator==(const env<C>& lhs, const env<C>& rhs)
//{
//  return lhs.ptr() == rhs.ptr();
//}
//
///// @internal
//template <typename C>
//inline
//bool
//operator<(const env<C>& lhs, const env<C>& rhs)
//{
//  return lhs.ptr() < rhs.ptr();
//}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

//namespace std {
//
///*------------------------------------------------------------------------------------------------*/
//
///// @brief Hash specialization for sdd::dd::internal_env
//template <typename Value, typename Successor>
//struct hash<sdd::dd::internal_env<Value, Successor>>
//{
//  std::size_t
//  operator()(const sdd::dd::internal_env<Value, Successor>& c)
//  const
//  {
//    using value_type     = typename sdd::dd::internal_env<Value, Successor>::value_type;
//    using successor_type = typename sdd::dd::internal_env<Value, Successor>::successor_type;
//
//    std::size_t seed = sdd::util::hash(c.level);
//    sdd::util::hash_combine(seed, c.values);
//    sdd::util::hash_combine(seed, c.successors);
//    return seed;
//  }
//};
//
///// @brief Hash specialization for sdd::dd::env
//template <typename C>
//struct hash<sdd::dd::env<C>>
//{
//  std::size_t
//  operator()(const sdd::dd::env<C>& c)
//  const noexcept
//  {
//    return sdd::util::hash(c.ptr());
//  }
//};
//
///*------------------------------------------------------------------------------------------------*/
//
//} // namespace std

#endif // _SDD_DD_PROTO_ENV_HH_
