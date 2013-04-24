#ifndef _SDD_DD_DEFINITION_HH_
#define _SDD_DD_DEFINITION_HH_

#include <initializer_list>
#include <type_traits> // is_integral

#include "sdd/dd/alpha.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/node.hh"
#include "sdd/dd/terminal.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/mem/variant.hh"
#include "sdd/order/order.hh"
#include "sdd/util/print_sizes_fwd.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief SDD at the deepest level.
template <typename C>
using flat_node = node<C, typename C::Values>;

/// @brief All but SDD at the deepest level.
template <typename C>
using hierarchical_node = node<C, SDD<C>>;

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Tag to describe the type of a node.
enum class node_tag {flat, hierarchical};

/// @internal
/// @brief Signature of the meta-function that returns the node's type corresponding to the
/// given tag.
template <typename C, enum node_tag>
struct node_for_tag;

/// @internal
/// @brief Specialization for flat node.
template <typename C>
struct node_for_tag<C, node_tag::flat>
{
  typedef flat_node<C> type;
};

/// @internal
/// @brief Specialization for hierarchical node.
template <typename C>
struct node_for_tag<C, node_tag::hierarchical>
{
  typedef hierarchical_node<C> type;
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Hierarchical Set Decision Diagram.
template <typename C>
class SDD final
{

  static_assert( std::is_integral<typename C::Variable>::value
               , "A variable must be an integral type.");

private:

  /// @brief A canonized SDD.
  ///
  /// This is the real recursive definition of an SDD: it can be a |0| or |1| terminal, or it
  /// can be a flat or an hierachical node.
  typedef mem::variant< const zero_terminal<C>, const one_terminal<C>
                      , const flat_node<C>, const hierarchical_node<C>>
          data_type;

  /// @brief A unified and canonized SDD, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  typedef mem::ref_counted<const data_type> unique_type;

  /// @brief Define the smart pointer around a unified SDD.
  ///
  /// It handles the reference counting as well as the deletion of the SDD when it is no longer
  /// referenced.
  typedef mem::ptr<const unique_type> ptr_type;

public:

  /// @brief The type of variables.
  typedef typename C::Variable variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The type of a value in a set of values.
  typedef typename C::Values::value_type value_type;

private:

  /// @brief The real smart pointer around a unified SDD.
  ptr_type ptr_;

public:

  /// @brief Move constructor.
  ///
  /// O(1).
  SDD(SDD&&) noexcept = default;

  /// @brief Move operator.
  ///
  /// O(1).
  SDD&
  operator=(SDD&&) noexcept = default;

  /// @brief Copy constructor.
  ///
  /// O(1).
  SDD(const SDD&) noexcept = default;

  /// @brief Copy operator.
  ///
  /// O(1).
  SDD&
  operator=(const SDD&) noexcept = default;

  /// @internal
  /// @brief  Construct a terminal.
  /// @param  terminal If true, create the |1| terminal; the |0| terminal otherwise.
  /// @return The terminal |0| or |1|.
  ///
  /// O(1).
  SDD(bool terminal)
    : ptr_(terminal ? one_ptr() : zero_ptr())
  {
  }

  /// @brief Construct a hierarchical SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values constructed from an initialization list.
  /// @param succ The SDD's successor.
  ///
  /// O(1), for the creation of the SDD itself, but the complexity of the construction of the
  /// set of values depends on values_type.
  SDD(const variable_type& var, std::initializer_list<value_type> values, const SDD& succ)
    : ptr_(create_node(var, values_type(values), SDD(succ)))
  {
  }

  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, values_type&& val, const SDD& succ)
    : ptr_(create_node(var, std::move(val), succ))
  {
  }

  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, const values_type& val, const SDD& succ)
    : ptr_(create_node(var, val, succ))
  {
  }

  /// @brief Construct a hierarchical SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, an SDD in this case.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, const SDD& val, const SDD& succ)
    : ptr_(create_node(var, val, succ))
  {
  }

  /// @brief Construct an SDD with an order.
  template <typename Initializer>
  SDD(const order<C>& o, const Initializer& init)
    : ptr_(one_ptr())
  {
    if (o.empty())
    {
      return;
    }
    // flat
    else if (o.nested().empty())
    {
      ptr_ = create_node(o.variable(), init(o.identifier()), SDD(o.next(), init));
    }
    // hierarchical
    else
    {
      ptr_ = create_node(o.variable(), SDD(o.nested(), init), SDD(o.next(), init));
    }
  }

  /// @brief  Indicate if the SDD is |0|.
  /// @return true if the SDD is |0|, false otherwise.
  ///
  /// O(1).
  bool
  empty()
  const noexcept
  {
    return ptr_ == zero_ptr();
  }

  /// @brief Swap two SDD.
  ///
  /// O(1).
  friend void
  swap(SDD& lhs, SDD& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.ptr_, rhs.ptr_);
  }

  /// @internal
  /// @brief Construct an SDD from a ptr.
  ///
  /// O(1).
  SDD(const ptr_type& ptr)
  noexcept
    : ptr_(ptr)
  {
  }

  /// @internal
  /// @brief Construct an SDD from a moved ptr.
  ///
  /// O(1).
  SDD(ptr_type&& ptr)
  noexcept
    : ptr_(std::move(ptr))
  {
  }

  /// @internal
  /// @brief  Construct an SDD, flat or hierarchical, with an alpha.
  /// \tparam Valuation If an SDD, constructs a hierarchical SDD; if a set of values,
  /// constructs a flat SDD.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  SDD(const variable_type& var, dd::alpha_builder<C, Valuation>&& builder)
    : ptr_(create_node(var, std::move(builder)))
  {
  }

  /// @internal
  /// @brief Get the content of the SDD (an mem::ref_counted).
  ///
  /// O(1).
  const unique_type&
  operator*()
  const noexcept
  {
    return *ptr_;
  }

  /// @internal
  /// @brief Get a pointer to the content of the SDD (an mem::ref_counted).
  ///
  /// O(1).
  const unique_type*
  operator->()
  const noexcept
  {
    return ptr_.operator->();
  }

  /// @internal
  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @internal
  /// @brief Create the |0| terminal.
private:

  ///
  /// O(1). The |0| is cached in a static variable.
  static
  ptr_type
  zero_ptr()
  {
    static char* addr = mem::allocate<unique_type>();
    static unique_type* z = new (addr) unique_type(mem::construct<zero_terminal<C>>());
    static const ptr_type zero(mem::unify(z));
    return zero;
  }

  /// @internal
  /// @brief Create the |1| terminal.
  ///
  /// O(1). The |1| is cached in a static variable.
  static
  ptr_type
  one_ptr()
  {
    static char* addr = mem::allocate<unique_type>();
    static unique_type* o = new (addr) unique_type(mem::construct<one_terminal<C>>());
    static const ptr_type one(mem::unify(o));
    return one;
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, Valuation&& val, const SDD& succ)
  {
    if (succ.empty() or val.empty())
    {
      return zero_ptr();
    }
    else
    {
      dd::alpha_builder<C, Valuation> builder;
      builder.add(std::move(val), succ);
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, const Valuation& val, const SDD& succ)
  {
    if (succ.empty() or val.empty())
    {
      return zero_ptr();
    }
    else
    {
      dd::alpha_builder<C, Valuation> builder;
      builder.add(val, succ);
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, dd::alpha_builder<C, Valuation>&& builder)
  {
    if (builder.empty())
    {
      return zero_ptr();
    }
    else
    {
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @internal
  /// @brief Helper function to unify a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  const unique_type&
  unify_node(const variable_type& var, dd::alpha_builder<C, Valuation>&& builder)
  {
    // Will be erased by the unicity table, either it's an already existing node or a deletion
    // is requested by ptr.
    // Note that the alpha function is allocated right behind the node, thus extra care must be
    // taken. This is also why we use Boost.Intrusive in order to be able to manage memory
    // exactly the way we want.
    char* addr = mem::allocate<unique_type>(builder.size_to_allocate());
    unique_type* u =
      new (addr) unique_type(mem::construct<node<C, Valuation>>(), var, builder);
    return mem::unify(u);
  }

  friend void util::print_sizes<C>(std::ostream&);
};

/*------------------------------------------------------------------------------------------------*/

/// @brief   Equality of two SDD.
/// @related SDD
///
/// O(1).
template <typename C>
inline
bool
operator==(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return lhs.ptr() == rhs.ptr();
}

/// @brief   Inequality of two SDD.
/// @related SDD
///
/// O(1).
template <typename C>
inline
bool
operator!=(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return not (lhs.ptr() == rhs.ptr());
}

/// @brief   Comparison of two SDD.
/// @related SDD
///
/// O(1). The order of SDD is arbitrary and can change at each run.
template <typename C>
inline
bool
operator<(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return lhs.ptr() < rhs.ptr();
}

/// @brief   Export the textual representation of an SDD to a stream.
/// @related SDD
///
/// Use only with small SDD, output can be huge.
template <typename C>
std::ostream&
operator<<(std::ostream& os, const SDD<C>& x)
{
  return os << x->data();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Return the |0| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
zero()
noexcept
{
  return {false};
}

/// @brief Return the |1| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
one()
noexcept
{
  return {true};
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::SDD.
template <typename C>
struct hash<sdd::SDD<C>>
{
  std::size_t
  operator()(const sdd::SDD<C>& x)
  const noexcept
  {
    return std::hash<decltype(x.ptr())>()(x.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_DEFINITION_HH_
