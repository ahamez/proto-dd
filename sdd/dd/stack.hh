#ifndef _SDD_DD_STACK_HH_
#define _SDD_DD_STACK_HH_

#include <algorithm>
#include <vector>

#include "sdd/dd/default_value.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
struct stack
{
  std::vector<T> elements;

  T
  operator[] (std::size_t i) const
  {
    if (i < this->elements.size())
      return this->elements[i];
    else
      return default_value<T>::value();
  }

  template <typename Shift>
  stack&
  shift(const stack& rhs, Shift&& sh)
  {
    std::size_t max_size = std::max(size(*this), size(rhs));
    this->elements.resize(max_size, default_value<T>::value());
    for (std::size_t i = 0; i != max_size; ++i)
      this->elements[i] = sh(this->elements[i], rhs[i]);
    return canonize(*this);
  }

  template <typename Rebuild>
  stack&
  rebuild(const stack& rhs, Rebuild&& rb)
  {
    std::size_t max_size = std::max(size(*this), size(rhs));
    this->elements.resize(max_size, default_value<T>::value());
    for (std::size_t i = 0; i != max_size; ++i)
      this->elements[i] = rb(this->elements[i], rhs[i]);
    return canonize(*this);
  }

  stack&
  pop()
  {
    if (not elements.empty())
    {
      for (std::size_t i = 1; i != elements.size(); ++i)
      {
        elements[i-1] = elements[i];
      }
      elements.pop_back();
    }
    return *this;
  }

};

template <typename T>
stack<T>
push (const stack<T>& s, const T& e)
{
  if (s.elements.empty() && (e == default_value<T>::value()))
    return s;
  else
  {
    stack<T> result;
    result.elements.reserve(s.elements.size() + 1);
    result.elements.push_back(e);
    std::copy(s.elements.cbegin(), s.elements.cend(), std::back_inserter(result.elements));
    return result;
  }
}

template <typename T>
T
head (const stack<T>& s)
{
  if (s.elements.empty())
    return default_value<T>::value();
  else
    return s.elements.front();
}

template <typename T>
std::ostream&
operator<< (std::ostream& os, const stack<T>& s)
{
  os << "[";
  for (const auto& x : s.elements)
    os << " " << x;
  return os << " ]";
}

template <typename T>
std::size_t
size (const stack<T>& s)
{
  return s.elements.size();
}

template <typename T>
stack<T>&
canonize (stack<T>& s)
{
  for (std::size_t i = s.elements.size() ; i > 0; --i)
  {
    if (s[i-1] == default_value<T>::value())
      s.elements.pop_back();
    else
      break;
  }
  s.elements.shrink_to_fit();
  return s;
}

template <typename T, typename Common>
stack<T>
common (const std::vector<std::reference_wrapper<const stack<T>>>& ss, Common&& cm)
{
  std::size_t max_size = 0;
  for (const auto& s : ss)
    max_size = std::max(max_size, size(s.get()));
  stack<T> result;
  result.elements.reserve(max_size);
  std::vector<T> values;
  values.reserve(ss.size());
  for (std::size_t i = 0; i != max_size; ++i)
  {
    for (const auto& s : ss)
      values.push_back(s.get()[i]);
    result.elements.push_back(cm(values.begin(), values.end()));
    values.clear();
  }
  return canonize(result);
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
