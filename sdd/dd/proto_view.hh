#ifndef _SDD_DD_PROTO_VIEW_HH_
#define _SDD_DD_PROTO_VIEW_HH_

#include <vector>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/proto_env.hh"
//#include "sdd/dd/node.hh"
#include "sdd/dd/proto_node.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
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

public:

  using values_type = typename C::Values;
  using valuation_type = values_type;
  using value_type = typename values_type::value_type;

  using env_type = dd::proto_env<C>;
  using arcs_type = std::vector<arc<C, values_type>>;

  /// @brief The type of the variable of this node.
  using variable_type = typename C::variable_type;

  /// @brief A (const) iterator on the arcs of this node.
  using const_iterator = typename arcs_type::const_iterator;


private:

  /// @brief
  ///
  /// A shared pointer behind the scene.
  const env_type env_;

  /// @brief Store the temporary arcs of this view.
  const arcs_type arcs_;

public:

  proto_view(const env_type& env, const proto_node<C>& node)
  noexcept
    : env_(env), arcs_(mk_arcs(env, node))
  {}

  /// @brief Get the variable of this node.
  ///
  /// O(1).
  variable_type
  variable()
  const noexcept
  {
    return env_.level();
  }

  /// @brief Get the beginning of arcs.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return arcs_.cbegin();
  }

  /// @brief Get the end of arcs.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return arcs_.cend();
  }

  /// @brief Get the number of arcs.
  ///
  /// O(1).
  std::size_t
  size()
  const noexcept
  {
    return arcs_.size();
  }

private:

  /// @todo On the fly generation of arcs
  /// @todo A cache?
  /// @todo stack::pop() doesn't need to be functional
  static
  arcs_type
  mk_arcs(const env_type& env, const proto_node<C>& node)
  {
    arcs_type arcs;
    arcs.reserve(node.arcs().size());

    // A buffer of values reused for each arc.
    std::vector<value_type> values_buffer;
    values_buffer.reserve(node.begin()->current_values.size() * 4);

    for (const auto& proto_arc : node)
    {
      // Rebuild the stacks needed to construct this arc.
      auto values_stack = proto_arc.values;
      values_stack.rebuild(env.values_stack(), C::rebuild);

      auto succs_stack = proto_arc.successors;
      succs_stack.rebuild(env.successors_stack(), [](const SDD<C>& lhs, const SDD<C>& rhs)
                                                    {
                                                      return rhs == zero<C>() ? lhs : rhs;
                                                    });

      // Get the values of the current level.
      const auto k = head(values_stack);
      std::transform( proto_arc.current_values.cbegin(), proto_arc.current_values.cend()
                    , std::back_inserter(values_buffer)
                    , [&](value_type v){return C::rebuild(v, k);});

      // Get the successor of the current level.
      const auto succ = head(succs_stack);

      // The current arc is complete.
      arcs.emplace_back( values_type(values_buffer.cbegin(), values_buffer.cend())
                       , SDD<C>(succ.ptr(), dd::proto_env<C>( env.level() - 1
                                                            , std::move(pop(values_stack))
                                                            , std::move(pop(succs_stack)))));

      // Will be re-used on next iteration.
      values_buffer.clear();
    }
    return arcs;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
inline
proto_view<C>
view(const proto_node<C>& n, const dd::proto_env<C>& env)
{
  return {env, n};
}

/*------------------------------------------------------------------------------------------------*/

/// @brief   Export a proto_view to a stream.
/// @related proto_view
template <typename C>
std::ostream&
operator<<(std::ostream& os, const proto_view<C>& n)
{
  return os << "proto_view TODO";
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
