#ifndef _SDD_DD_PROTO_VIEW_HH_
#define _SDD_DD_PROTO_VIEW_HH_

#include <vector>

//#include "sdd/dd/alpha.hh"
#include "sdd/dd/proto_env.hh"
#include "sdd/dd/node.hh"
//#include "sdd/dd/proto_node.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
class proto_view final
{
//  // Can't copy a proto_view.
//  proto_view& operator=(const proto_view&) = delete;
//  proto_view(const proto_view&) = delete;
//
//  // Can't move a proto_view.
//  proto_view& operator=(proto_view&&) = delete;
//  proto_view(proto_view&&) = delete;
//
//public:
//
//  using values_type = typename C::Values;
//  using value_type = typename values_type::value_type;
//
//  using env_type = dd::env<C>;
//  using arcs_type = std::vector<arc<C, values_type>>;
//
//  /// @brief The type of the variable of this node.
//  using variable_type = typename C::Variable;
//
//  /// @brief A (const) iterator on the arcs of this node.
//  using const_iterator = typename arcs_type::const_iterator;
//
//private:
//
//
//  const env_type env_;
//
//  const arcs_type arcs_;
//
//public:
//
//  proto_view(const env_type& env, const proto_node<C>& node)
//  noexcept
//    : env_(env), arcs_(mk_arcs(env, node))
//  {}
//
//  /// @brief Get the variable of this node.
//  ///
//  /// O(1).
//  variable_type
//  variable()
//  const noexcept
//  {
//    return env_.level();
//  }
//
//  /// @brief Get the beginning of arcs.
//  ///
//  /// O(1).
//  const_iterator
//  begin()
//  const noexcept
//  {
//    return arcs_.cbegin();
//  }
//
//  /// @brief Get the end of arcs.
//  ///
//  /// O(1).
//  const_iterator
//  end()
//  const noexcept
//  {
//    return arcs_.cend();
//  }
//
//  /// @brief Get the number of arcs.
//  ///
//  /// O(1).
//  std::size_t
//  size()
//  const noexcept
//  {
//    return arcs_.size();
//  }
//
//private:
//
//  static
//  arcs_type
//  mk_arcs(const env_type& env, const proto_node<C>& node)
//  {
//    typename dd::env<C>::value_stack_type value_stack;
//    typename dd::env<C>::successor_stack_type successor_stack;
//
//    arcs_type arcs;
//    arcs.reserve(node.arcs().size());
//
//    const auto height = env.level() + 1;
//
//    const auto env_k = *env.value_stack().limit(height).begin();
//    const auto env_succ = *env.successor_stack().limit(height).begin();
//
//    // A value buffer
//    std::vector<value_type> values_buffer;
//    values_buffer.reserve(node.begin()->current_values.size() * 4);
//
//    for (const auto& proto_arc : node)
//    {
//      // Iterators for the env's sparse stacks.
//      auto env_k_cit = env.value_stack().limit(height - 1).rbegin();
//      auto env_succs_cit = env.successor_stack().limit(height - 1).rbegin();
//
//      // Iterators for the current proto arc's sparse stacks.
//      auto arc_r_cit = proto_arc.phi.limit(height).rbegin();
//      auto arc_succs_cit = proto_arc.psi.limit(height).rbegin();
//
//      // Rebuild values of the current arc.
//      std::transform( proto_arc.current_values.cbegin(), proto_arc.current_values.cend()
//                    , std::inserter(values_buffer, values_buffer.begin())
//                    , [&](value_type v){return C::rebuild(v, env_k);});
//
//      // Get the successor of the current arc.
//      const auto succ = env_succ == zero<C>() ? *arc_succs_cit : env_succ;
//
//      // Rebuild phi of successors
//      for (std::size_t i = 1; i < height; ++i, ++arc_r_cit, ++env_k_cit)
//      {
//        value_stack.push(C::rebuild(*arc_r_cit, *env_k_cit));
//      }
//
//      // Rebuild psi of successors
//      for (std::size_t i = 1; i < height; ++i, ++arc_succs_cit, ++env_succs_cit)
//      {
//        successor_stack.push( *env_succs_cit == zero<C>()
//                            ? arc_succs_cit
//                            : *env_succs_cit);
//      }
//
//      arcs.emplace_back( values_type(values_buffer.cbegin(), values_buffer.cend())
//                       , SDD<C>(succ.ptr(), dd::env<C>( env.level() - 1
//                                                      , std::move(value_stack)
//                                                      , std::move(successor_stack))));
//    }
//
//    return arcs;
//  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
const node<C, typename C::Values>&
view(const proto_node<C>& n, const dd::proto_env<C>& env)
{
//  return n;
  return reinterpret_cast<const node<C, typename C::Values>&>(n);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief   Export a proto_view to a stream.
/// @related proto_view
template <typename C>
std::ostream&
operator<<(std::ostream& os, const proto_view<C>& n)
{
//  // +n.variable(): widen the type. It's useful to print the values of char and unsigned char types.
//  os << +n.variable() << "[";
//  std::for_each( n.begin(), n.end() - 1
//               , [&](const arc<C, typename proto_view<C>::valuation_type>& a)
//                    {os << a.valuation() << " --> " << a.successor() << " || ";});
//  return os << (n.end() - 1)->valuation() << " --> " << (n.end() - 1)->successor() << "]";
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_PROTO_VIEW_HH_
