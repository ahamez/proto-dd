#ifndef _SDD_HOM_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_HH_

#include <algorithm> // any_of, copy, find
#include <iterator>  // back_insert
#include <iosfwd>
#include <initializer_list>
#include <memory>    // make_shared, shared_ptr, unique_ptr
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/expression.hh"
#include "sdd/hom/expression/simple.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

#if !defined(HAS_NO_BOOST_COROUTINE)
/// @internal
/// @brief expression homomorphism.
template <typename C>
class _expression
{
public:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const order_positions_type positions_;

  /// @brief The target of the assignment.
  const order_position_type target_;

public:

  /// @brief Constructor.
  _expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr, order_positions_type&& positions
             , order_position_type target)
    : eval_ptr_(std::move(e_ptr)), positions_(std::move(positions)), target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target_ and o.position() != positions_.front()
       and not o.contains(o.position(), positions_.front())
       and not o.contains(o.position(), target_);
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& sdd)
  const
  {
    std::shared_ptr<expr::app_stack<C>> app = nullptr;
    std::shared_ptr<expr::res_stack<C>> res = nullptr;
    expr::expression_pre<C> eval {cxt, target_, *eval_ptr_};
    return visit_self(eval, sdd, o, app, res, positions_.cbegin(), positions_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const order_positions_type&
  operands()
  const noexcept
  {
    return positions_;
  }

  /// @brief Get the target.
  order_position_type
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _expression
template <typename C>
inline
bool
operator==(const _expression<C>& lhs, const _expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.operands() == rhs.operands()
     and lhs.evaluator() == rhs.evaluator();
}

/// @internal
/// @related _expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _expression<C>& e)
{
  os << "expression(" << e.target() << " = ";
  e.evaluator().print(os);
  return os << ")";
}
#endif // !defined(HAS_NO_BOOST_COROUTINE)

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Simple expression homomorphism.
template <typename C>
class _simple_expression
{
public:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const order_positions_type positions_;

  /// @brief The target of the assignment.
  const order_position_type target_;

public:

  /// @brief Constructor.
  _simple_expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr
                    , order_positions_type&& positions, order_position_type target)
    : eval_ptr_(std::move(e_ptr)), positions_(std::move(positions)), target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target_ and o.position() != positions_.front()
       and not o.contains(o.position(), positions_.front())
       and not o.contains(o.position(), target_);
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& sdd)
  const
  {
    std::shared_ptr<expr::app_stack<C>> app = nullptr;
    std::shared_ptr<expr::res_stack<C>> res = nullptr;
    expr::simple<C> eval {cxt, target_, *eval_ptr_};
    return visit_self(eval, sdd, o, app, res, positions_.cbegin(), positions_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const order_positions_type&
  operands()
  const noexcept
  {
    return positions_;
  }

  /// @brief Get the target.
  order_position_type
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _simple_expression
template <typename C>
inline
bool
operator==(const _simple_expression<C>& lhs, const _simple_expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.operands() == rhs.operands()
     and lhs.evaluator() == rhs.evaluator();
}

/// @internal
/// @related expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _simple_expression<C>& e)
{
  os << "SimpleExpr(" << e.target() << " = ";
  e.evaluator().print(os);
  return os << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the expression homomorphism.
/// @related homomorphism
/// @todo How to handle the dumb case where there is only one identifier, which is also the target?
///
/// Elements of [begin, end) must be unique.
template <typename C, typename Evaluator, typename InputIterator>
homomorphism<C>
expression( const order<C>& o, const Evaluator& u, InputIterator begin, InputIterator end
          , const typename C::Identifier& target)
{
  using identifier_type = typename C::Identifier;
  using derived_type = hom::expr::evaluator_derived<C, Evaluator>;

  const std::size_t size = std::distance(begin, end);

  if (size == 0)
  {
    return id<C>();
  }

  const auto target_pos = o.node(target).position();

  order_positions_type positions;
  positions.reserve(size);
  std::transform( begin, end, std::back_inserter(positions)
                , [&](const identifier_type& id){return o.node(id).position();});
  std::sort(positions.begin(), positions.end());


  std::unique_ptr<derived_type> evaluator_ptr(new derived_type(u));

  const auto last_position = positions.back();
  if (target_pos < last_position)
  {
#if !defined(HAS_NO_BOOST_COROUTINE)
    return homomorphism<C>::create( mem::construct<hom::_expression<C>>()
                                  , std::move(evaluator_ptr), std::move(positions), target_pos);
#else
    throw std::runtime_error("Can't create full expressions without Boost.Coroutine.");
#endif
  }
  else
  {
    // The target is below all operands, it's a much simpler case to handle
    return homomorphism<C>::create( mem::construct<hom::_simple_expression<C>>()
                                  , std::move(evaluator_ptr), std::move(positions), target_pos);
  }
}

/// @brief Create the expression homomorphism.
/// @related homomorphism
///
/// Elements of ids must be unique.
template <typename C, typename Evaluator>
homomorphism<C>
expression( const order<C>& o, const Evaluator& u, std::initializer_list<typename C::Identifier> ids
          , const typename C::Identifier& target)
{
  return expression(o, u, ids.cbegin(), ids.cend(), target);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

#if !defined(HAS_NO_BOOST_COROUTINE)
/// @internal
/// @brief Hash specialization for sdd::hom::_expression.
template <typename C>
struct hash<sdd::hom::_expression<C>>
{
  std::size_t
  operator()(const sdd::hom::_expression<C>& e)
  const
  {
    std::size_t seed = e.evaluator().hash();
    sdd::util::hash_combine(seed, e.operands().begin(), e.operands().end());
    sdd::util::hash_combine(seed, e.target());
    return seed;
  }
};
#endif // !defined(HAS_NO_BOOST_COROUTINE)

/// @internal
/// @brief Hash specialization for sdd::hom::s_imple_expression.
template <typename C>
struct hash<sdd::hom::_simple_expression<C>>
{
  std::size_t
  operator()(const sdd::hom::_simple_expression<C>& e)
  const noexcept
  {
    std::size_t seed = e.evaluator().hash();
    sdd::util::hash_combine(seed, e.operands().begin(), e.operands().end());
    sdd::util::hash_combine(seed, e.target());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_EXPRESSION_HH_
