#ifndef _SDD_ORDER_ORDER_HH_
#define _SDD_ORDER_ORDER_HH_

#include <algorithm> // find
#include <initializer_list>
#include <iostream>
#include <memory>    // shared_ptr
#include <sstream>
#include <utility>   // pair
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "sdd/order/order_builder.hh"
#include "sdd/order/order_node.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
using order_positions_type = std::vector<order_position_type>;

/// @internal
using order_positions_iterator = typename order_positions_type::const_iterator;

/*------------------------------------------------------------------------------------------------*/

/// @brief Represent an order of identifiers, possibly with some hierarchy.
/// @todo Manage artificial identifiers.
///
/// It helps associate a variable (generated by the library) in an SDD to an identifiers
/// (provided to the user). An identifier should appear only once by order.
template <typename C>
class order final
{
public:

  /// @brief A user's identifier type.
  using identifier_type = typename C::Identifier;

  /// @brief A library's variable type.
  using variable_type = typename C::Variable;

private:

  /// @brief A path, following hierarchies, to a node.
  using path_type = typename order_node<C>::path_type;

  /// @brief All nodes.
  using nodes_type = std::vector<order_node<C>>;

  /// @brief A shared pointer to nodes.
  using nodes_ptr_type = std::shared_ptr<const nodes_type>;

  /// @brief Define a mapping identifier->node.
  using id_to_node_type = std::unordered_map<identifier_type, const order_node<C>*>;

  /// @brief The concrete order.
  const nodes_ptr_type nodes_ptr_;

  /// @brief Maps identifiers to nodes.
  const std::shared_ptr<id_to_node_type> id_to_node_ptr_;

  /// @brief The first node in the order.
  const order_node<C>* head_;

public:

  /// @brief Constructor.
  order(const order_builder<C>& builder)
    : nodes_ptr_(mk_nodes_ptr(builder))
    , id_to_node_ptr_(mk_identifier_to_node(nodes_ptr_))
    , head_(nodes_ptr_ ? &(nodes_ptr_->front()) : nullptr)
  {}

  /// @brief Tell if upper contains nested in its possibly contained hierarchy.
  /// @param uppper Must belong to the current order.
  /// @param nested Must belong to the current order.
  bool
  contains(order_position_type upper, order_position_type nested)
  const noexcept
  {
    const auto& path = (*nodes_ptr_)[nested].path();
    return std::find(path.begin(), path.end(), upper) != path.end();
  }

  /// @brief
  const nodes_type&
  identifiers()
  const noexcept
  {
    return *nodes_ptr_;
  }

  /// @brief Get the variable of this order's head.
  variable_type
  variable()
  const noexcept
  {
    return head_->variable();
  }

  /// @brief Get the identifier of this order's head.
  const identifier_type&
  identifier()
  const noexcept
  {
    return head_->identifier();
  }

  /// @brief Get the position of this order's head.
  order_position_type
  position()
  const noexcept
  {
    return head_->position();
  }

  /// @brief Get the next order of this order's head.
  order
  next()
  const noexcept
  {
    return order(nodes_ptr_, id_to_node_ptr_, head_->next());
  }

  /// @brief Get the nested order of this order's head.
  order
  nested()
  const noexcept
  {
    return order(nodes_ptr_, id_to_node_ptr_, head_->nested());
  }

  /// @brief Tell if this order is empty.
  bool
  empty()
  const noexcept
  {
    return head_ == nullptr;
  }

  /// @internal
  const order_node<C>&
  node(const identifier_type& id)
  const noexcept
  {
    return *id_to_node_ptr_->at(id);
  }

  /// @internal
  std::size_t
  hash()
  const noexcept
  {
    std::size_t seed = 0;
    util::hash_combine(seed, nodes_ptr_.get());
    util::hash_combine(seed, head_);
    return seed;
  }

  /// @internal
  bool
  operator==(const order& other)
  const noexcept
  {
    return nodes_ptr_ == other.nodes_ptr_ and id_to_node_ptr_ == other.id_to_node_ptr_
      and head_ == other.head_;
  }

private:

  /// @brief Construct whith a shallow copy an already existing order.
  order( const nodes_ptr_type& nodes_ptr, const std::shared_ptr<id_to_node_type>& id_to_node
       , const order_node<C>* head)
    : nodes_ptr_(nodes_ptr), id_to_node_ptr_(id_to_node), head_(head)
  {}

  /// @brief Create the concrete order using an order_builder.
  static
  nodes_ptr_type
  mk_nodes_ptr(const order_builder<C>& builder)
  {
    if (builder.empty())
    {
      return nullptr;
    }

    auto nodes_ptr = std::make_shared<nodes_type>(builder.size());
    auto& nodes = *nodes_ptr;

    // Ensure that identifiers appear only once.
    std::unordered_set<identifier_type> unicity;

    // Counter for identifiers' positions.
    unsigned int pos = 0;

    // To enable recursion in the lambda.
    std::function<
      std::pair<order_node<C>*, variable_type>
      (const order_builder<C>&, const std::shared_ptr<path_type>&)
    > helper;

    helper = [&helper, &pos, &nodes, &unicity]
    (const order_builder<C>& ob, const std::shared_ptr<path_type>& path)
    -> std::pair<order_node<C>*, unsigned int>
    {
      const unsigned int current_position = pos++;

      std::pair<order_node<C>*, variable_type> nested(nullptr, 0 /*first variable*/);
      std::pair<order_node<C>*, variable_type> next(nullptr, 0 /* first variable */);

      if (not ob.nested().empty())
      {
        const auto new_path = std::make_shared<path_type>(*path);
        new_path->push_back(current_position);
        new_path->shrink_to_fit();
        nested = helper(ob.nested(), new_path);
      }

      if (not ob.next().empty())
      {
        next = helper(ob.next(), path);
      }

      const auto variable = next.second;
      if (not unicity.insert(ob.identifier()).second)
      {
        std::stringstream ss;
        ss << "Duplicate identifier " << ob.identifier() << " when constructing order";
        throw std::runtime_error(ss.str());
      }
      nodes[current_position] =
        order_node<C>(ob.identifier(), variable, current_position, next.first, nested.first, path);
      return std::make_pair(&nodes[current_position], variable + 1);
    };

    // Launch the recursion.
    helper(builder, std::make_shared<path_type>());

    return nodes_ptr;
  }

  /// @brief
  const std::shared_ptr<id_to_node_type>
  mk_identifier_to_node(const nodes_ptr_type& nodes_ptr)
  {
    auto identifier_to_node_ptr = std::make_shared<id_to_node_type>();
    if (nodes_ptr)
    {
      identifier_to_node_ptr->reserve(nodes_ptr->size());
      std::for_each( nodes_ptr->begin(), nodes_ptr_->end()
                   , [&](const order_node<C>& n)
                        {identifier_to_node_ptr->emplace(n.identifier(), &n);}
                   );
    }
    return identifier_to_node_ptr;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Textual representation of an order.
/// @related order
template <typename C>
std::ostream&
operator<<(std::ostream& os, const order<C>& ord)
{
  const std::function<std::ostream&(const order<C>&, unsigned int)> helper
   = [&helper, &os](const order<C>& o, unsigned int indent)
  -> std::ostream&
  {
    if (not o.empty())
    {
      const std::string spaces(indent, ' ');
      os << spaces << o.identifier() << std::endl;
      if (not o.nested().empty())
      {
        helper(o.nested(), indent + 2);
      }
      if (not o.next().empty())
      {
        helper(o.next(), indent);
      }
    }
    return os;
  };
  return helper(ord, 0);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::order.
template <typename C>
struct hash<sdd::order<C>>
{
  std::size_t
  operator()(const sdd::order<C>& o)
  const noexcept
  {
    return o.hash();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_ORDER_ORDER_HH_
