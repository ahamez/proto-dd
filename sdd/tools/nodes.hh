#ifndef _SDD_TOOLS_NODES_HH_
#define _SDD_TOOLS_NODES_HH_

#include <unordered_set>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct nb_nodes_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = std::pair<unsigned int, unsigned int>;

  /// @brief How to identify a node.
  using node_id_type = typename proto_view<C>::id_type;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  mutable std::unordered_set<node_id_type> visited_;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&)
  const
  {
    return std::make_pair(0, 0);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&)
  const
  {
    return std::make_pair(0, 0);
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    if (visited_.emplace(n.id()).second)
    {
      result_type res {1, 0};
      for (const auto& arc : n)
      {
        accumulate_pair(res, visit(*this, arc.successor()));
      }
      return res;
    }
    else
    {
      return std::make_pair(0, 0);
    }
  }

private:

  static
  void
  accumulate_pair(result_type& lhs, result_type&& rhs)
  {
    lhs.first += rhs.first;
    lhs.second += rhs.second;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the number of unique nodes of an SDD.
///
/// Each node is counted only once.
template <typename C>
std::pair<unsigned int, unsigned int>
nodes(const SDD<C>& x)
{
  return visit(nb_nodes_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_NODES_HH_
