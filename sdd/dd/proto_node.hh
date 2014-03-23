#ifndef _SDD_DD_PROTO_NODE_HH_
#define _SDD_DD_PROTO_NODE_HH_

#include <algorithm>  // equal, for_each
#include <functional> // hash
#include <iosfwd>
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/dd/stack.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct proto_arc
{
  using values_type = typename C::Values;
  using value_type = typename values_type::value_type;

  values_type current_values;
  dd::stack<value_type> values;
  dd::stack<SDD<C>> successors;

  proto_arc(values_type&& values)
    : current_values(std::move(values)), values(), successors()
  {}
};

template <typename C>
inline
bool
operator==(const proto_arc<C>& lhs, const proto_arc<C>& rhs)
noexcept
{
  return lhs.current_values == rhs.current_values and lhs.values == rhs.values
     and lhs.successors == rhs.successors;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief  A non-terminal node in an SDD.
///
/// For the sake of canonicity, a node shall not exist in several locations, thus we prevent
/// its copy. Also, to enforce this canonicity, we need that nodes have always the same
/// address, thus they can't be moved to an other memory location once created.
template <typename C>
class proto_node final
{
  // Can't copy a proto_node.
  proto_node& operator=(const proto_node&) = delete;
  proto_node(const proto_node&) = delete;

  // Can't move a proto_node.
  proto_node& operator=(proto_node&&) = delete;
  proto_node(proto_node&&) = delete;

public:

  /// @brief The type of the valuation of this node.
  using values_type = typename C::Values;

  using value_type = typename values_type::value_type;

  using arcs_type = std::vector<proto_arc<C>>;

  using const_iterator = typename arcs_type::const_iterator;

private:

  const arcs_type arcs_;

public:

  proto_node(arcs_type&& arcs)
    : arcs_(std::move(arcs))
  {}

  /// @brief Get the beginning of arcs.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return arcs_.begin();
  }

  /// @brief Get the end of arcs.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return arcs_.end();
  }

  const arcs_type&
  arcs()
  const noexcept
  {
    return arcs_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Equality of two proto_node.
/// @related proto_node
///
/// O(1) if nodes don't have the same number of arcs; otherwise O(n) where n is the number of
/// arcs.
template <typename C>
inline
bool
operator==(const proto_node<C>& lhs, const proto_node<C>& rhs)
noexcept
{
  return lhs.arcs() == rhs.arcs();
}

/// @brief Export a proto_node to a stream.
/// @related proto_node
template <typename C>
std::ostream&
operator<<(std::ostream& os, const proto_node<C>& n)
{
  return os << "proto_node TODO";
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::proto_arc
template <typename C>
struct hash<sdd::proto_arc<C>>
{
  std::size_t
  operator()(const sdd::proto_arc<C>& n)
//  const noexcept(noexcept(sdd::util::hash(n.begin(), n.end())))
  const
  {
    std::size_t seed = sdd::util::hash(n.current_values);
    sdd::util::hash_combine(seed, n.values);
    sdd::util::hash_combine(seed, n.successors);
    return seed;
  }
};

/// @brief Hash specialization for sdd::proto_node
template <typename C>
struct hash<sdd::proto_node<C>>
{
  std::size_t
  operator()(const sdd::proto_node<C>& n)
  const noexcept(noexcept(sdd::util::hash(n.begin(), n.end())))
  {
    return sdd::util::hash(n.begin(), n.end());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_PROTO_NODE_HH_
