#ifndef _SDD_DD_PROTO_VIEW_HH_
#define _SDD_DD_PROTO_VIEW_HH_

#include <vector>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/proto_env.hh"
#include "sdd/dd/proto_node.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help identify a proto_view.
template <typename C>
struct proto_view_identity
{
  using env_type = dd::proto_env<C, SDD<C>>;

  const char* env;
//  const env_type& env;
  const char* node;

  proto_view_identity(const env_type& env, const proto_node<C>& node)
  noexcept
    : env(reinterpret_cast<const char*>(&(*env)))
//    : env(env)
    , node(reinterpret_cast<const char*>(&node))
  {}

  bool
  operator==(const proto_view_identity& other)
  const noexcept
  {
    return env == other.env and node == other.node;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class proto_view final
{
  // Can't copy a proto_view.
  proto_view& operator=(const proto_view&) = delete;
  proto_view(const proto_view&) = delete;

public:

  using values_type = typename C::Values;
  using valuation_type = values_type;
  using value_type = typename values_type::value_type;

  using env_type = dd::proto_env<C, SDD<C>>;
  using arc_type = arc<C, values_type>;
  using arcs_type = std::vector<arc_type>;

  /// @brief The type of the variable of this node.
  using variable_type = typename C::variable_type;

  /// @brief A (const) iterator on the arcs of this node.
  using const_iterator = typename arcs_type::const_iterator;

  using id_type = proto_view_identity<C>;

private:

  /// @brief
  ///
  /// A shared pointer behind the scene.
  const env_type env_;

  /// @brief Store the temporary arcs of this view.
  const arcs_type arcs_;

  /// @brief Keep the original proto_node for identifications purposes.
  const proto_node<C>& node_;

public:

  proto_view(const env_type& env, const proto_node<C>& node)
  noexcept
    : env_(env), arcs_(mk_arcs(env, node)), node_(node)
  {}

  // Move a proto_view.
  proto_view& operator=(proto_view&&) = default;
  proto_view(proto_view&&) = default;

  /// @brief Get the variable of this node.
  ///
  /// O(1).
  variable_type
  variable()
  const noexcept
  {
    return env_.level() - 1;
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

  /// @brief Get an value that uniquely identify any proto_view created with the same environment
  /// and proto_node.
  id_type
  id()
  const noexcept
  {
    return proto_view_identity<C>(env_, node_);
  }

private:

  /// @todo On the fly generation of arcs
  /// @todo A cache?
  /// @todo stack::pop() doesn't need to be functional
  static
  arcs_type
  mk_arcs(const env_type& env, const proto_node<C>& node)
  {
    assert((env.level() - 1) < env.level() && "Overflow");
    assert(node.arcs().size() >= 1 && "Empty proto_node");

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
                       , SDD<C>(succ.ptr(), env_type( env.level() - 1
                                                    , std::move(pop(values_stack))
                                                    , std::move(pop(succs_stack)))));

      // Will be re-used on next iteration.
      values_buffer.clear();
    }
    return arcs;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C, typename Successor>
inline
proto_view<C>
view(const proto_node<C>& n, const dd::proto_env<C, Successor>& env)
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

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::proto_view_identity
template <typename C>
struct hash<sdd::proto_view_identity<C>>
{
  std::size_t
  operator()(const sdd::proto_view_identity<C>& id)
  const noexcept
  {
    std::size_t seed = sdd::util::hash(id.env);
    sdd::util::hash_combine(seed, id.node);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_PROTO_VIEW_HH_
