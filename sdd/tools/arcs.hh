#ifndef _SDD_TOOLS_ARCS_HH_
#define _SDD_TOOLS_ARCS_HH_

#include <unordered_map>
#include <unordered_set>
#include <utility> // pair

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

using arcs_frequency_type
  = std::unordered_map< std::size_t /*number of arcs*/
                      , std::pair< std::size_t /* flat arcs frequency */
                                 , std::size_t /* hierarchical arcs frequency */>>;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct arcs_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = void;

  /// @brief How to identify a node.
  using node_id_type = typename flat_node<C>::id_type;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  mutable std::unordered_set<node_id_type> visited_;

  /// @brief Stores the frequency of apparition of a number of arcs.
  mutable arcs_frequency_type map_;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&)
  const
  {}

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&)
  const
  {}

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    if (visited_.emplace(n.id()).second)
    {
      map_[n.size()].first += 1;
      for (const auto& arc : n)
      {
        visit(*this, arc.successor());
      }
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the arc frequency map of an SDD.
///
/// An arc frequency map indicates the number of nodes with 1, 2, etc. arcs.
template <typename C>
arcs_frequency_type
arcs(const SDD<C>& x)
{
  arcs_visitor<C> visitor;
  visit(visitor, x);
  return std::move(visitor.map_);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the total number of arcs from an arc frequency map.
std::pair<unsigned int /* flat arcs */, unsigned int /* hierarchical arcs */>
number_of_arcs(const arcs_frequency_type& freq)
noexcept
{
  std::pair<unsigned int, unsigned int> res {0, 0};
  for (const auto& kv : freq)
  {
    res.first += kv.first * kv.second.first;
    res.second += kv.first * kv.second.second;
  }
  return res;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_ARCS_HH_
