#ifndef _SDD_DD_COUNT_COMBINATIONS_HH_
#define _SDD_DD_COUNT_COMBINATIONS_HH_

#include <unordered_map>

#include "sdd/dd/count_combinations_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/values/size.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Visitor to count the number of paths in an SDD.
template <typename C>
struct count_combinations_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = boost::multiprecision::cpp_int;

  /// @brief How to identify a node.
  using node_id_type = typename flat_node<C>::id_type;

  /// @brief A cache is used to speed up the computation.
  mutable std::unordered_map<node_id_type, result_type> cache_;

  /// @brief Error case.
  ///
  /// We should not encounter any |0| as all SDD leading to |0| are reduced to |0| and as
  /// the |0| alone is treated in count_combinations.
  result_type
  operator()(const zero_terminal<C>&)
  const noexcept
  {
    assert(false && "Encountered the |0| terminal when counting paths.");
    __builtin_unreachable();
  }

  /// @brief Terminal case of the recursion.
  result_type
  operator()(const one_terminal<C>&)
  const noexcept
  {
    return 1;
  }

  /// @brief The number of paths for a flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    const auto insertion = cache_.emplace(n.id(), 0);
    if (insertion.second)
    {
      for (const auto& arc : n)
      {
        insertion.first->second += size(arc.valuation()) * visit(*this, arc.successor());
      }
    }
    return insertion.first->second;
  }

};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Compute the number of combinations in an SDD.
///
/// O(N) where N is the number of nodes in x.
template <typename C>
inline
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x)
{
  return x.empty() ? 0 : visit(count_combinations_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // _SDD_DD_COUNT_COMBINATIONS_HH_
