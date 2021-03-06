#ifndef _SDD_HOM_INTERSECTION_HH_
#define _SDD_HOM_INTERSECTION_HH_

#include <algorithm>  // all_of, copy
#include <deque>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>  //invalid_argument
#include <unordered_map>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/dd/top.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/interrupt.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief intersection homomorphism.
template <typename C>
class _intersection
{
public:

  /// @brief The type of the homomorphism operands' set.
  using operands_type = boost::container::flat_set<homomorphism<C>>;

  /// @brief The type of a const iterator on this intersection's operands.
  using const_iterator = typename operands_type::const_iterator;

private:

  /// @brief The homomorphism operands' set.
  const operands_type operands_;

public:

  /// @brief Constructor.
  _intersection(operands_type&& operands)
    : operands_(std::move(operands))
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    dd::intersection_builder<C, SDD<C>> intersection_operands;
    intersection_operands.reserve(operands_.size());
    try
    {
      for (const auto& op : operands_)
      {
        try
        {
          intersection_operands.add(op(cxt, o, x));
        }
        catch (interrupt<C>& i)
        {
          intersection_operands.add(i.result());
          i.result() = dd::intersection(cxt.sdd_context(), std::move(intersection_operands));
          throw;
        }
      }
      return dd::intersection(cxt.sdd_context(), std::move(intersection_operands));
    }
    catch (top<C>& t)
    {
      evaluation_error<C> e(x);
      e.add_top(t);
      throw e;
    }
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return std::all_of( operands_.begin(), operands_.end()
                      , [&o](const homomorphism<C>& h){return h.skip(o);});
  }


  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return std::all_of( operands_.begin(), operands_.end()
                      , [](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get an iterator to the first operand.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return operands_.begin();
  }

  /// @brief Get an iterator to the end of operands.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return operands_.end();
  }

  const operands_type&
  operands()
  const noexcept
  {
    return operands_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _intersection
template <typename C>
inline
bool
operator==(const _intersection<C>& lhs, const _intersection<C>& rhs)
noexcept
{
  return lhs.operands() == rhs.operands();
}

/// @internal
/// @related _intersection
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _intersection<C>& s)
{
  os << "(";
  std::copy( s.operands().begin(), std::prev(s.operands().end())
           , std::ostream_iterator<homomorphism<C>>(os, " & "));
  return os << *std::prev(s.operands().end()) << ")";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help optimize an intersection's operands.
template <typename C>
struct intersection_builder_helper
{
  using result_type   = void;
  using operands_type = typename _intersection<C>::operands_type;
  using hom_list_type = std::deque<homomorphism<C>> ;
  using locals_type   = std::unordered_map<order_position_type, hom_list_type>;

  operands_type& operands_;
  locals_type& locals_;

  intersection_builder_helper(operands_type& operands, locals_type& locals)
  noexcept
    : operands_(operands)
    , locals_(locals)
  {}

  /// @brief Flatten nested intersections.
  void
  operator()(const _intersection<C>& s, const homomorphism<C>&)
  const
  {
    for (const auto& op : s.operands())
    {
      visit_self(*this, op);
    }
  }

  /// @brief Regroup locals.
  void
  operator()(const _local<C>& l, const homomorphism<C>&)
  const
  {
    auto insertion = locals_.emplace(l.target(), hom_list_type());
    insertion.first->second.emplace_back(l.hom());
  }

  /// @brief Insert normally all other operands.
  template <typename T>
  void
  operator()(const T&, const homomorphism<C>& h)
  const
  {
    operands_.insert(h);
  }

};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the intersection homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
intersection(const order<C>& o, InputIterator begin, InputIterator end)
{
  const auto size = std::distance(begin, end);

  if (size == 0)
  {
    throw std::invalid_argument("Empty operands at intersection construction.");
  }

  typename hom::_intersection<C>::operands_type operands;
  operands.reserve(size);

  typename hom::intersection_builder_helper<C>::locals_type locals;
  hom::intersection_builder_helper<C> ib {operands, locals};
  for (; begin != end; ++begin)
  {
    visit_self(ib, *begin);
  }

  // insert remaining locals
  for (const auto& l : locals)
  {
    operands.insert(local<C>(l.first, intersection(o, l.second.begin(), l.second.end())));
  }

  if (operands.size() == 1)
  {
    return *operands.begin();
  }
  else
  {
    operands.shrink_to_fit();
    return homomorphism<C>::create(mem::construct<hom::_intersection<C>>(), std::move(operands));
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the intersection homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
intersection(const order<C>& o, std::initializer_list<homomorphism<C>> operands)
{
  return intersection(o, operands.begin(), operands.end());
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_intersection.
template <typename C>
struct hash<sdd::hom::_intersection<C>>
{
  std::size_t
  operator()(const sdd::hom::_intersection<C>& s)
  const
  {
    return sdd::util::hash(s.operands().begin(), s.operands().end());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_INTERSECTION_HH_
