#ifndef _SDD_DD_INTERSECTION_HH_
#define _SDD_DD_INTERSECTION_HH_

#include <boost/container/flat_set.hpp>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/nary.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/dd/square_union.hh"
#include "sdd/util/hash.hh"
#include "sdd/values/empty.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/// @internal
/// @brief The intersection operation in the cache.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED intersection_op_impl
{
  /// @brief The textual representation of the intersection operator.
  static constexpr char symbol = '&';

  /// @brief Perform the SDD intersection algorithm.
  template <typename InputIterator, typename NodeType>
  static
  SDD<C>
  work(InputIterator begin, InputIterator end, context<C>& cxt)
  {
    using node_type      = NodeType;
    using valuation_type = typename node_type::valuation_type;
    using variable_type  = typename node_type::variable_type;

    auto operands_cit = begin;
    const auto operands_end = end;

    // Result accumulator, initialized with the first operand.
    SDD<C> res = *operands_cit;

    const variable_type variable = operands_cit->env().level();


    // We re-use the same square union to save some allocations.
    square_union<C, valuation_type> su;

    for (++operands_cit; operands_cit != operands_end; ++operands_cit)
    {
      // Throw a Top if operands are incompatible (different types or different variables).
      check_compatibility(res, *operands_cit);

      // Cleanup for the next usage.
      su.clear();

      const auto lhs = res.view();
      const auto rhs = operands_cit->view();

      for (auto& lhs_arc : lhs)
      {
        for (auto& rhs_arc : rhs)
        {
          intersection_builder<C, valuation_type> valuation_builder;
          valuation_builder.add(lhs_arc.valuation());
          valuation_builder.add(rhs_arc.valuation());
          valuation_type inter_val = intersection(cxt, std::move(valuation_builder));

          if (not values::empty_values(inter_val))
          {
            SDD<C> inter_succ = intersection(cxt, {lhs_arc.successor(), rhs_arc.successor()});

            if (not values::empty_values(inter_succ))
            {
              su.add(inter_succ, std::move(inter_val));
            }
          }
        }
      }

      // Exit as soon as an intermediary result is empty.
      if (su.empty())
      {
        return zero<C>();
      }

      /// @todo avoid to create an intermediary SDD at each loop.
      res = SDD<C>(variable, su(cxt));
    }

    return res;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Add an arc to the operands of the intersection operation.
///
/// This implementation is meant to be used as a policy by nary_builder which doesn't know how
/// to add an arc.
template <typename C, typename Valuation>
struct intersection_builder_impl
{
  /// @brief Tell if a zero is contained in this set of operands.
  bool has_zero;

  /// @brief Constructor.
  intersection_builder_impl()
    : has_zero(false)
  {}

  /// @brief Add an rvalue operand.
  template <typename Container>
  void
  add(Container& set, Valuation&& operand)
  {
    if (has_zero)
    {
      return;
    }
    if (values::empty_values(operand))
    {
      has_zero = true,
      set.clear();
      return;
    }
    set.insert(std::move(operand));
  }

  /// @brief Add an lvalue operand.
  template <typename Container>
  void
  add(Container& set, const Valuation& operand)
  {
    if (has_zero)
    {
      return;
    }
    if (values::empty_values(operand))
    {
      has_zero = true,
      set.clear();
      return;
    }
    set.insert(operand);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The intersection operation applied on SDD.
template <typename C>
inline
SDD<C>
intersection(context<C>& cxt, intersection_builder<C, SDD<C>>&& builder)
{
  if (builder.empty())
  {
    return zero<C>();
  }
  else if (builder.size() == 1)
  {
    return *builder.begin();
  }
  return cxt.intersection_cache()(intersection_op<C>(builder));
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The intersection operations applied on sets of values.
template <typename C, typename Values>
inline
Values
intersection(context<C>&, intersection_builder<C, Values>&& builder)
{
  if (builder.empty())
  {
    return Values();
  }
  else if (builder.size() == 1)
  {
    return *builder.begin();
  }
  else
  {
    auto cit = builder.begin();
    const auto end = builder.end();
    Values result = *cit;
    for (++cit; cit != end and not values::empty_values(result); ++cit)
    {
      typename C::Values tmp = intersection(result, *cit);
      using std::swap;
      swap(tmp, result);
    }
    return result;
  }
}

} // namespace dd

/*------------------------------------------------------------------------------------------------*/

/// @brief Perform the intersection of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
operator&(const SDD<C>& lhs, const SDD<C>& rhs)
{
  return dd::intersection(global<C>().sdd_context, {lhs, rhs});
}

/// @brief Perform the intersection of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>&
operator&=(SDD<C>& lhs, const SDD<C>& rhs)
{
  SDD<C> tmp = dd::intersection(global<C>().sdd_context, {lhs, rhs});
  using std::swap;
  swap(tmp, lhs);
  return lhs;
}

/// @brief Perform the intersection of an iterable container of SDD.
/// @related SDD
template <typename C, typename InputIterator>
SDD<C>
inline
intersection(InputIterator begin, InputIterator end)
{
  dd::intersection_builder<C, SDD<C>> builder;
  for (; begin != end; ++begin)
  {
    builder.add(*begin);
  }
  return dd::intersection(global<C>().sdd_context, std::move(builder));
}

/// @brief Perform the intersection of an initializer list of SDD.
/// @related SDD
template <typename C>
SDD<C>
inline
intersection(std::initializer_list<SDD<C>> operands)
{
  return intersection<C>(std::begin(operands), std::end(operands));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_INTERSECTION_HH_
