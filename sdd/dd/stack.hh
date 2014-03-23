#ifndef _SDD_DD_STACK_HH_
#define _SDD_DD_STACK_HH_

#include <algorithm> // copy
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

  stack<T>&
  operator -= (const stack<T>& rhs)
  {
    std::size_t max_size = std::max(size(*this), size(rhs));
    this->elements.resize(max_size, default_value<T>::value());
    for (int i = 0; i != max_size; ++i)
      this->elements[i] -= rhs[i];
    return canonize(*this);
  }

  stack<T>&
  operator += (const stack<T>& rhs)
  {
    std::size_t max_size = std::max(size(*this), size(rhs));
    this->elements.resize(max_size, default_value<T>::value());
    for (int i = 0; i != max_size; ++i)
      this->elements[i] += rhs[i];
    return canonize(*this);
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
    for (const auto& x : s.elements)
      result.elements.push_back(x);
    return result;
  }
}

template <typename T>
stack<T>
pop (const stack<T>& s)
{
  if (s.elements.empty())
    return s;
  else
  {
    stack<T> result;
    result.elements.reserve(s.elements.size() - 1);
    for (int i = 1; i != s.elements.size(); ++i)
      result.elements.push_back(s.elements[i]);
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
  for (int i = s.elements.size() ; i > 0; --i)
  {
    if (s[i-1] == default_value<T>::value())
      s.elements.pop_back();
    else
      break;
  }
  s.elements.shrink_to_fit();
  return s;
}

template <typename T>
stack<T>
common (const std::vector<stack<T>>& ss)
{
  std::size_t max_size = 0;
  for (const auto& s : ss)
    max_size = std::max(max_size, size(s));
  stack<T> result;
  result.elements.reserve(max_size);
  for (int i = 0; i != max_size; ++i)
  {
    std::vector<T> values;
    for (const auto& s : ss)
      values.push_back(s[i]);
    result.elements.push_back(*std::min_element(values.begin(), values.end()));
  }
  return canonize(result);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief   Equality of two stack.
/// @related stack
template <typename T>
inline
bool
operator==(const stack<T>& lhs, const stack<T>& rhs)
noexcept
{
  return lhs.elements == rhs.elements;
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
    return sdd::util::hash(s.elements.cbegin(), s.elements.cend());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_SPARSE_STACK_HH_
