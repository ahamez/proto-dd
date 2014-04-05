#ifndef _SDD_DD_STACK_HH_
#define _SDD_DD_STACK_HH_

#include <algorithm>
#include <vector>

#include "sdd/dd/default_value.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
struct stack
{
  std::vector<T> elements;

  const T&
  operator[] (std::size_t i)
  const noexcept
  {
    return elements[i];
  }

  template <typename Shift>
  stack&
  shift(const stack& rhs, Shift&& sh)
  noexcept(noexcept(sh(std::declval<T>(), std::declval<T>())))
  {
    auto rhs_cit = rhs.elements.cbegin();
    std::for_each(elements.begin(), elements.end(), [&](T& x){x = sh(x, *rhs_cit++);});
    return *this;
  }

  template <typename Rebuild>
  stack&
  rebuild(const stack& rhs, Rebuild&& rb)
  noexcept(noexcept(rb(std::declval<T>(), std::declval<T>())))
  {
    auto rhs_cit = rhs.elements.cbegin();
    std::for_each(elements.begin(), elements.end(), [&](T& x){x = rb(x, *rhs_cit++);});
    return *this;
  }

  stack&
  pop()
  noexcept
  {
    elements.pop_back();
    return *this;
  }
};

/// @internal
template <typename T, typename E>
inline
stack<T>
push(stack<T> s, E&& e)
{
  s.elements.push_back(std::move(e));
  return s;
}

/// @internal
template <typename T>
inline
const T&
head(const stack<T>& s)
noexcept
{
  return s.elements.back();
}

/// @internal
template <typename T>
std::ostream&
operator<< (std::ostream& os, const stack<T>& s)
{
  os << "[";
  for (const auto& x : s.elements)
    os << " " << x;
  return os << " ]";
}

/// @internal
template <typename T>
std::vector<T>&
static_values_buffer()
{
  static struct initializer
  {
    std::vector<T> vec;
    initializer() : vec() {vec.reserve(1024);}
  } init;
  return init.vec;
}

/// @internal
template <typename T, typename Common>
stack<T>
common(const std::vector<std::reference_wrapper<const stack<T>>>& ss, Common&& cm)
{
  auto& values_buffer = static_values_buffer<T>();
  stack<T> res;
  const auto size = ss.begin()->get().elements.size();
  res.elements.reserve(size);

  for (auto i = 0; i < size; ++i)
  {
    for (const auto& s : ss)
    {
      values_buffer.push_back(s.get()[i]);
    }
    res.elements.push_back(cm(values_buffer.begin(), values_buffer.end()));
    values_buffer.clear();
  }
  return res;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two stack.
/// @related stack
template <typename T>
inline
bool
operator==(const stack<T>& lhs, const stack<T>& rhs)
noexcept
{
  return lhs.elements == rhs.elements;
}

/// @internal
/// @brief Comparison of two stack.
/// @related stack
template <typename T>
inline
bool
operator<(const stack<T>& lhs, const stack<T>& rhs)
noexcept
{
  return lhs.elements < rhs.elements;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::stack
template <typename T>
struct hash<sdd::dd::stack<T>>
{
  std::size_t
  operator()(const sdd::dd::stack<T>& s)
  const
  {
    if (s.elements.empty())
    {
      return sdd::util::hash(0);
    }
    else
    {
      return sdd::util::hash(s.elements.cbegin(), s.elements.cend());
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_SPARSE_STACK_HH_
