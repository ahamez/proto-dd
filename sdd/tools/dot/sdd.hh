#ifndef _SDD_DD_TOOLS_DOT_SDD_HH_
#define _SDD_DD_TOOLS_DOT_SDD_HH_

#include <unordered_map>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = unsigned int;

  /// @brief How to identify a node.
  using node_id_type = typename proto_view<C>::id_type;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  mutable std::unordered_map<node_id_type, unsigned int> cache_;

  mutable unsigned int last_id_;

  /// @brief The stream to export to.
  std::ostream& os_;

  /// @brief Constructor.
  to_dot_visitor(std::ostream& os)
    : cache_(), last_id_(1), os_(os)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>& n)
  const
  {
    assert(false);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>& n)
  const
  {
    return 1;
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    auto insertion = cache_.emplace(n.id(), 0);
    if (insertion.second)
    {
      const auto id = ++last_id_;
      insertion.first->second = id;
      os_ << "node_" << id << " [label=\"" << +n.variable() << "\"];" << std::endl;
      for (const auto& arc : n)
      {
        const auto succ = visit(*this, arc.successor());
        os_ << "node_" << id << " -> " << "node_" << succ
            << " [label=\"" << arc.valuation() << "\"];"
            << std::endl;
      }
    }
    return insertion.first->second;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot
{
  const SDD<C> x_;

  to_dot(const SDD<C>& x)
    : x_(x)
  {}

  friend
  std::ostream&
  operator<<(std::ostream& out, const to_dot& manip)
  {
    out << "digraph sdd {" << std::endl;
    if (manip.x_.empty())
    {
      out << "node_0 [shape=square,label=\"0\"];" << std::endl;
    }
    else
    {
      out << "node_1 [shape=square,label=\"1\"];" << std::endl;
      visit(to_dot_visitor<C>(out), manip.x_);
    }
    return out << "}" << std::endl;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export an SDD to the DOT format.
template <typename C>
to_dot<C>
dot(const SDD<C>& x)
{
  return to_dot<C>(x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_DD_TOOLS_DOT_SDD_HH_
