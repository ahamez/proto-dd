#ifndef _SDD_DD_DEFINITION_HH_
#define _SDD_DD_DEFINITION_HH_

#include <algorithm> // all_of
#include <cassert>
#include <tuple>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/alpha.hh"
#include "sdd/dd/count_combinations_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/node.hh"
#include "sdd/dd/path_generator_fwd.hh"
#include "sdd/dd/proto_env.hh"
#include "sdd/dd/proto_node.hh"
#include "sdd/dd/proto_view_fwd.hh"
#include "sdd/dd/terminal.hh"
#include "sdd/dd/top.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/mem/variant.hh"
#include "sdd/order/order.hh"
#include "sdd/values/empty.hh"
#include "sdd/values/values_traits.hh"

// Include for forwards at the end of the file.

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief SDD at the deepest level.
//template <typename C>
//using flat_node = node<C, typename C::Values>;
template <typename C>
using flat_node = proto_view<C>;

///// @brief All but SDD at the deepest level.
//template <typename C>
//using hierarchical_node = node<C, SDD<C>>;

template <typename C> SDD<C> zero() noexcept;
template <typename C> SDD<C> one() noexcept;

/*------------------------------------------------------------------------------------------------*/

/// @brief Hierarchical Set Decision Diagram.
template <typename C>
class SDD
{
private:

  /// @brief A canonized SDD.
  ///
  /// This is the real recursive definition of an SDD: it can be a |0| or |1| terminal, or it
  /// can be a flat or an hierachical node.
  typedef mem::variant<zero_terminal<C>, one_terminal<C>, proto_node<C>> data_type;

public:

  /// @internal
  static constexpr std::size_t zero_terminal_index
    = data_type::template index_for_type<zero_terminal<C>>();

  /// @internal
  static constexpr std::size_t one_terminal_index
    = data_type::template index_for_type<one_terminal<C>>();

  /// @internal
  static constexpr std::size_t proto_node_index
    = data_type::template index_for_type<proto_node<C>>();

//  /// @internal
//  static constexpr std::size_t hierarchical_node_index
//    = data_type::template index_for_type<hierarchical_node<C>>();

  /// @internal
  /// @brief A unified and canonized SDD, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  using unique_type = mem::ref_counted<data_type>;

  /// @internal
  /// @brief The type of the smart pointer around a unified SDD.
  ///
  /// It handles the reference counting as well as the deletion of the SDD when it is no longer
  /// referenced.
  using ptr_type = mem::ptr<unique_type>;

  /// @internal
  /// @brief The type of the environment used to reconstruct a flat SDD.
  using proto_env_type = dd::proto_env<C, SDD>;

  /// @brief The type of variables.
  using variable_type = typename C::variable_type;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Convenient type to store both proto environment and the real SDD.
  using tuple_type = std::tuple<proto_env_type, ptr_type>;

private:

  /// @brief The environment to reconstruct the corresponding flat SDD.
  proto_env_type env_;

  /// @brief The real smart pointer around a unified SDD.
  ptr_type ptr_;

public:

  /// @brief Default constructor.
  SDD()
    : env_(dd::empty_proto_env<C, SDD>())
    , ptr_(zero_ptr())
  {}

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
  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(variable_type var, values_type&& val, const SDD& succ)
    : env_(dd::empty_proto_env<C, SDD>())
//    , ptr_(create_node(var, std::move(val), succ))
    , ptr_(zero_ptr())
  {
    std::tie(env_, ptr_) = create_node(var, std::move(val), succ);
  }

  /// @internal
  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(variable_type var, const values_type& val, const SDD& succ)
    : env_(dd::empty_proto_env<C, SDD>())
//    , ptr_(create_node(var, val, succ))
    , ptr_(zero_ptr())
  {
    std::tie(env_, ptr_) = create_node(var, val, succ);
  }

//  /// @internal
//  /// @brief Construct a hierarchical SDD.
//  /// @param var  The SDD's variable.
//  /// @param val  The SDD's valuation, an SDD in this case.
//  /// @param succ The SDD's successor.
//  ///
//  /// O(1).
//  SDD(variable_type var, const SDD& val, const SDD& succ)
//    : env_(dd::empty_proto_env<C>())
//    , ptr_(create_node(var, val, succ))
//  {}

  /// @brief Construct an SDD with an order.
  template <typename Initializer>
  SDD(const order<C>& o, const Initializer& init)
    : env_(dd::empty_proto_env<C, SDD>())
    , ptr_(one_ptr())
  {
    if (o.empty()) // base case of the recursion, ptr_ is defaulted to |1|
    {
      return;
    }
    else if (o.nested().empty()) // flat
    {
      // We can safely pass the order_identifier as a user one because only hierarchical levels
      // can be artificial.
      assert(not o.identifier().artificial());
//      ptr_ = create_node(o.variable(), init(o.identifier().user()), SDD(o.next(), init));
      std::tie(env_, ptr_)
        = create_node(o.variable(), init(o.identifier().user()), SDD(o.next(), init));
    }
    else // hierarchical
    {
//      ptr_ = create_node(o.variable(), SDD(o.nested(), init), SDD(o.next(), init));
      assert(false && "Hierarchy in order.");
    }
  }

#if !defined(HAS_NO_BOOST_COROUTINE)
  /// @brief Return an iterable object which generates all paths of this SDD.
  path_generator<C>
  paths()
  const
  {
    boost::coroutines::attributes attrs(boost::coroutines::fpu_not_preserved);
    return path_generator<C>(std::bind(dd::paths<C>, std::placeholders::_1, *this), attrs);
  }
#endif

  /// @brief Indicate if the SDD is |0|.
  /// @return true if the SDD is |0|, false otherwise.
  ///
  /// O(1).
  bool
  empty()
  const noexcept
  {
    return ptr_ == zero_ptr(); // and env() == dd::empty_proto_env<C>() ???
  }

  /// @brief Swap two SDD.
  ///
  /// O(1).
  friend void
  swap(SDD& lhs, SDD& rhs)
  noexcept
  {
    std::swap(lhs.env_, rhs.env_);
    std::swap(lhs.ptr_, rhs.ptr_);
  }

//  /// @internal
//  /// @brief Construct an SDD from a ptr.
//  ///
//  /// O(1).
//  SDD(const ptr_type& ptr)
//  noexcept
//    : env_(dd::empty_proto_env<C>())
//    , ptr_(ptr)
//  {}

  /// @internal
  SDD(const ptr_type& ptr, const proto_env_type& env)
  noexcept
    : env_(env)
    , ptr_(ptr)
  {}

  /// @internal
  /// @brief  Construct an SDD, flat or hierarchical, with an alpha.
  /// @tparam Valuation If an SDD, constructs a hierarchical SDD; if a set of values,
  /// constructs a flat SDD.
  ///
  /// O(n) where n is the number of arcs in the builder.
//  template <typename Valuation>
//  SDD(const variable_type& var, dd::alpha_builder<C, Valuation>&& builder)
  SDD(variable_type var, dd::alpha_builder<C, values_type>&& builder)
    : env_(dd::empty_proto_env<C, SDD>())
    , ptr_(zero_ptr())
  {
    std::tie(env_, ptr_) = create_node(var, std::move(builder));
  }

  /// @internal
  /// @brief Get the content of the SDD (an mem::variant).
  ///
  /// O(1).
  const data_type&
  operator*()
  const noexcept
  {
    return ptr_->data();
  }

  /// @internal
  /// @brief Get a pointer to the content of the SDD (an mem::variant).
  ///
  /// O(1).
  const data_type*
  operator->()
  const noexcept
  {
    return &ptr_->data();
  }

  /// @internal
  const proto_env_type&
  env()
  const noexcept
  {
    return env_;
  }

  /// @internal
  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  const ptr_type&
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @internal
  /// @brief Return the globally cached |0| terminal.
  ///
  /// O(1).
  static
  ptr_type
  zero_ptr()
  noexcept
  {
    return global<C>().zero;
  }

  /// @internal
  /// @brief Return the globally cached |1| terminal.
  ///
  /// O(1).
  static
  ptr_type
  one_ptr()
  noexcept
  {
    return global<C>().one;
  }

  /// @internal
  std::size_t
  index()
  const noexcept
  {
    return ptr_->data().index();
  }

  /// @brief Get the number of combinations stored in this SDD.
  boost::multiprecision::cpp_int
  size()
  const
  {
    return dd::count_combinations(*this);
  }

  /// @internal
  proto_view<C>
  view()
  const
  {
    assert(index() == proto_node_index && "Attempt to convert a non-proto_node");
    return proto_view<C>(env_, mem::variant_cast<proto_node<C>>(ptr_->data()));
  }

private:

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
//  template <typename Valuation>
  static
//  ptr_type
//  create_node(variable_type var, Valuation&& val, const SDD& succ)
  tuple_type
  create_node(variable_type var, values_type&& val, const SDD& succ)
  {
    if (succ.empty() or values::empty_values(val))
    {
//      return zero_ptr();
      return std::make_tuple(dd::empty_proto_env<C, SDD>(), zero_ptr());
    }
    else
    {
      dd::alpha_builder<C, values_type> builder;
      builder.add(std::move(val), succ);
//      return ptr_type(unify_node(var, std::move(builder)));
      return unify_proto(std::move(builder));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
//  template <typename Valuation>
  static
//  ptr_type
//  create_node(variable_type var, const Valuation& val, const SDD& succ)
  tuple_type
  create_node(variable_type var, const values_type& val, const SDD& succ)
  {
    if (succ.empty() or values::empty_values(val))
    {
      return std::make_tuple(dd::empty_proto_env<C, SDD>(), zero_ptr());
    }
    else
    {
      dd::alpha_builder<C, values_type> builder;
      builder.add(val, succ);
//      return ptr_type(unify_node(var, std::move(builder)));
      return unify_proto(std::move(builder));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
//  template <typename Valuation>
  static
//  ptr_type
//  create_node(variable_type var, dd::alpha_builder<C, Valuation>&& builder)
  tuple_type
  create_node(variable_type var, dd::alpha_builder<C, values_type>&& builder)
  {
    if (builder.empty())
    {
      return std::make_tuple(dd::empty_proto_env<C, SDD>(), zero_ptr());
    }
    else
    {
//      return ptr_type(unify_node(var, std::move(builder)));
      return unify_proto(std::move(builder));
    }
  }

  /// @internal
  static
  tuple_type
  unify_proto(dd::alpha_builder<C, values_type>&& builder)
  {
    using value_stack_type = typename proto_env_type::value_stack_type;
    using successor_stack_type = typename proto_env_type::successor_stack_type;
    using value_type = typename C::Values::value_type;

    // Compute the new level (level 1 is above |1|).
    const auto new_level = builder.begin()->first == one<C>()
                         ? 1
                         : builder.begin()->first.env().level() + 1;

    // The resulting canonized arcs.
    typename proto_node<C>::arcs_type arcs;
    arcs.reserve(builder.size());

    // Arcs' values stacks.
    std::vector<std::reference_wrapper<const typename proto_arc<C>::value_stack_type>>
      arcs_values_stacks;
    arcs_values_stacks.reserve(builder.size());

    // Arcs' successors stacks.
    std::vector<std::reference_wrapper<const typename proto_arc<C>::successor_stack_type>>
      arcs_succs_stacks;
    arcs_succs_stacks.reserve(builder.size());

    // Intialize all proto arcs
    for (const auto& sdd_values : builder)
    {
      typename values::values_traits<values_type>::builder values_builder;

      const auto k = C::common(sdd_values.second.cbegin(), sdd_values.second.cend());
      for (const auto& v : sdd_values.second)
      {
        values_builder.insert(C::shift(v, k));
      }

      // Construct arc of the proto_dd.
      arcs.emplace_back( values_type(std::move(values_builder))
                       , push(sdd_values.first.env().values_stack(), k)
                       , push( sdd_values.first.env().successors_stack()
                             , sdd_values.first
                             ));

      // Get a reference to this arc's stacks.
      arcs_values_stacks.push_back(arcs.back().values);
      arcs_succs_stacks.push_back(arcs.back().successors);
    }

    // Get the value stack to put in environment.
    value_stack_type env_value_stack
      = dd::common( arcs_values_stacks
                  , C::template common<std::vector<unsigned int>::const_iterator>);

    using sdd_cit = typename std::vector<SDD<C>>::const_iterator;
    successor_stack_type env_succs_stack
      = dd::common( arcs_succs_stacks
                  , [](sdd_cit begin, sdd_cit end)
                      {
                        return std::all_of(begin, end, [&](const SDD<C>& x){return x == *begin;})
                             ? *begin
                             : zero<C>();
                      });

    // Shift stacks on proto arcs with the new environments' stacks
    for (auto& proto_arc : arcs)
    {
      proto_arc.values.shift(env_value_stack, C::shift);
      proto_arc.successors.shift(env_succs_stack, [](const SDD<C>& lhs, const SDD<C>& rhs)
                                                    {
                                                      return rhs == zero<C>() ? lhs : rhs;
                                                    });
    }

    // Finally, we can create and unify the proto_dd.
    auto& ut = global<C>().sdd_unique_table;
    char* addr = ut.allocate(sizeof(proto_node<C>));
    unique_type* u =
      new (addr) unique_type(mem::construct<proto_node<C>>(), std::move(arcs));
    return std::make_tuple( proto_env_type( new_level
                                          , std::move(env_value_stack)
                                          , std::move(env_succs_stack))
                          , ptr_type(ut(u)));
  }
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
  return lhs.ptr() == rhs.ptr() and lhs.env() == rhs.env();
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
  return not (lhs == rhs);
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
  if (lhs.ptr() < rhs.ptr())
  {
    return true;
  }
  else if (lhs.ptr() == rhs.ptr())
  {
    return lhs.env() < rhs.env();
  }
  else
  {
    return false;
  }
}

/// @brief   Export the textual representation of an SDD to a stream.
/// @related SDD
///
/// Use only with small SDD, output can be huge.
template <typename C>
std::ostream&
operator<<(std::ostream& os, const SDD<C>& x)
{
  return os << *x;
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
  return {SDD<C>::zero_ptr(), dd::empty_proto_env<C, SDD<C>>()};
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
  return {SDD<C>::one_ptr(), dd::empty_proto_env<C, SDD<C>>()};
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related SDD
template <typename C>
std::size_t
check_compatibility(const SDD<C>& lhs, const SDD<C>& rhs)
{
  const auto lhs_index = lhs.index();
  const auto rhs_index = rhs.index();

  if (lhs_index != rhs_index)
  {
    // different type of nodes
    throw top<C>(lhs, rhs);
  }

//  typename SDD<C>::variable_type lhs_variable;
//  typename SDD<C>::variable_type rhs_variable;

//  // we must convert to the right type before comparing variables
//  if (lhs_index == SDD<C>::proto_node_index)
//  {
//    lhs_variable = mem::variant_cast<proto_node<C>>(*lhs).variable();
//    rhs_variable = mem::variant_cast<proto_node<C>>(*rhs).variable();
//  }
//  else
//  {
//    lhs_variable = mem::variant_cast<hierarchical_node<C>>(*lhs).variable();
//    rhs_variable = mem::variant_cast<hierarchical_node<C>>(*rhs).variable();
//  }

//  if (lhs_variable != rhs_variable)
//  {
//    throw top<C>(lhs, rhs);
//  }


  if (lhs_index == SDD<C>::proto_node_index)
  {
    if (lhs.env().level() != rhs.env().level())
    {
      throw top<C>(lhs, rhs);
    }
  }

  return lhs_index;
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
    std::size_t seed = sdd::util::hash(x.ptr());
    sdd::util::hash_combine(seed, x.env());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#include "sdd/dd/count_combinations.hh"
#include "sdd/dd/path_generator.hh"
#include "sdd/dd/proto_view.hh"

#endif // _SDD_DD_DEFINITION_HH_
