#ifndef _SDD_VALUES_FLAT_SET_HH_
#define _SDD_VALUES_FLAT_SET_HH_

#include <algorithm>  // copy, set_difference, set_intersection, set_union
#include <functional> // hash
#include <initializer_list>
#include <iosfwd>
#include <iterator>   // inserter
#include <utility>    // pair

#include "sdd/values_manager_fwd.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/util/boost_flat_set_no_warnings.hh"
#include "sdd/util/hash.hh"
#include "sdd/values/values_traits.hh"

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @brief A unified set of values, implemented with a sorted vector.
template <typename Value>
class flat_set final
{
public:

  /// @brief The type of the contained value.
  using value_type = Value;

  /// @internal
  /// @brief How to hash a boost::container::flat_set.
  struct hash_type
  {
    std::size_t
    operator()(const boost::container::flat_set<value_type>& fs)
    const noexcept
    {
      std::size_t seed = 0;
      for (const auto& x : fs)
      {
        sdd::util::hash_combine(seed, x);
      }
      return seed;
    }
  };

  /// @internal
  /// @brief The type of the real container.
  using data_type = boost::container::flat_set<value_type>;

  /// @internal
  using unique_type = mem::ref_counted<data_type, hash_type>;

  /// @internal
  using ptr_type = mem::ptr<unique_type>;

  /// @brief The type of an iterator on a flat set of values.
  using const_iterator = typename data_type::const_iterator;

private:

  /// @brief A pointer to the unified set of values.
  ptr_type ptr_;

public:

  /// @brief Default copy constructor.
  flat_set(const flat_set&) = default;

  /// @brief Default copy operator.
  flat_set& operator=(const flat_set&) = default;

  /// @brief Default constructor.
  flat_set()
    : ptr_(empty_set())
  {}

  /// @brief Constructor with a range.
  template <typename InputIterator>
  flat_set(InputIterator begin, InputIterator end)
    : ptr_(create(begin, end))
  {}

  /// @brief Constructor with a initializer_list.
  flat_set(std::initializer_list<Value> values)
    : flat_set(values.begin(), values.end())
  {}

  /// @brief Constructor from a temporary data_type.
  flat_set(data_type&& fs)
    : ptr_(create(std::move(fs)))
  {}

  /// @brief Insert a value.
  std::pair<const_iterator, bool>
  insert(const Value& x)
  {
    data_type fs(ptr_->data());
    const auto insertion = fs.insert(x);
    if (insertion.second)
    {
      fs.shrink_to_fit();
      ptr_ = create(std::move(fs));
    }
    return insertion;
  }

  /// @brief Get the beginning of this set of values.
  const_iterator
  begin()
  const noexcept
  {
    return ptr_->data().cbegin();
  }

  /// @brief Get the end of this set of values.
  const_iterator
  end()
  const noexcept
  {
    return ptr_->data().cend();
  }

  /// @brief Get the beginning of this set of values.
  const_iterator
  cbegin()
  const noexcept
  {
    return ptr_->data().cbegin();
  }

  /// @brief Get the end of this set of values.
  const_iterator
  cend()
  const noexcept
  {
    return ptr_->data().cend();
  }

  /// @brief Tell if this set of values is empty.
  bool
  empty()
  const noexcept
  {
    return ptr_->data().empty();
  }

  /// @brief Get the number of contained values.
  std::size_t
  size()
  const noexcept
  {
    return ptr_->data().size();
  }

  /// @brief Find a value.
  const_iterator
  find(const Value& x)
  const
  {
    return ptr_->data().find(x);
  }

  /// @brief Erase a value.
  std::size_t
  erase(const Value& x)
  {
    data_type d(ptr_->data());
    const std::size_t nb_erased = d.erase(x);
    ptr_ = create(std::move(d));
    return nb_erased;
  }

private:

  /// @brief Recursive implementation of erase_keys when only one value is left.
  template <typename Head>
  void
  erase_keys_impl(data_type& d, Head&& val)
  {
    d.erase(val);
  }

  /// @brief Recursive implementation of erase_keys.
  template <typename Head, typename... Tail>
  void
  erase_keys_impl(data_type& d, Head&& val, Tail&&... tail)
  {
    d.erase(val);
    erase_keys_impl<Tail...>(d, std::forward<Tail>(tail)...);
  }

public:

  /// @brief Erase several values.
  template <typename... Values>
  void
  erase_keys(Values&&... values)
  {
    data_type d(ptr_->data());
    erase_keys_impl(d, std::forward<Values>(values)...);
    ptr_ = create(std::move(d));
  }

  /// @brief
  const_iterator
  lower_bound(const Value& x)
  const
  {
    return ptr_->data().lower_bound(x);
  }

  /// @internal
  /// @brief Get the pointer to the unified data.
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @internal
  static
  ptr_type
  empty_set()
  {
    return global_values<flat_set<Value>>().state.empty;
  }

private:

  /// @brief Create a smart pointer to unified set of values, from a pair of iterators.
  template <typename InputIterator>
  static
  ptr_type
  create(InputIterator begin, InputIterator end)
  {
    if (begin == end)
    {
      return empty_set();
    }
    else
    {
      return ptr_type(unify(data_type(begin, end)));
    }
  }

  /// @brief Create a smart pointer to a unified set of values, from a boost::container::flat_set.
  static
  ptr_type
  create(data_type&& x)
  {
    if (x.empty())
    {
      return empty_set();
    }
    else
    {
      return ptr_type(unify(std::move(x)));
    }
  }

  /// @brief Return the unfied version of a boost::container::flat_set.
  static
  unique_type&
  unify(data_type&& x)
  {
    auto& ut = global_values<flat_set<Value>>().state.unique_table;
    char* addr = ut.allocate(0 /*extra bytes*/);
    unique_type* u = new (addr) unique_type(std::move(x));
    return ut(u);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Will be used by sdd::manager.
template <typename Value>
struct flat_set_manager
{
  /// @brief The type of a unified flat_set.
  using unique_type = typename flat_set<Value>::unique_type;

  /// @brief The type of smart pointer to a unified flat_set.
  using ptr_type = typename flat_set<Value>::ptr_type;

  /// @brief Manage the handler needed by ptr when a unified data is no longer referenced.
  struct ptr_handler
  {
    ptr_handler(mem::unique_table<unique_type>& ut)
    {
      mem::set_deletion_handler<unique_type>([&](const unique_type& u){ut.erase(u);});
    }

    ~ptr_handler()
    {
      mem::reset_deletion_handler<unique_type>();
    }
  } handler;

  /// @brief The set of unified flat_set.
  mem::unique_table<unique_type> unique_table;

  /// @brief The cached empty flat_set.
  const ptr_type empty;

  /// @brief Constructor.
  template <typename C>
  flat_set_manager(const C& configuration)
    : handler(unique_table)
    , unique_table(configuration.flat_set_unique_table_size)
    , empty(mk_empty())
  {}

private:

  /// @brief Helper to construct the empty flat_set.
  ptr_type
  mk_empty()
  {
    char* addr = unique_table.allocate(0 /*extra bytes*/);
    unique_type* u = new (addr) unique_type;
    return ptr_type(unique_table(u));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related flat_set
///
/// Indicate to the library that flat_set needs to store a global state.
template <typename Value>
struct values_traits<flat_set<Value>>
{
  static constexpr bool stateful = true;
  static constexpr bool fast_iterable = true;
  using state_type = flat_set_manager<Value>;
  using builder = boost::container::flat_set<Value>;
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Equality of flat_set
/// @related flat_set
///
/// O(1).
template <typename Value>
inline
bool
operator==(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  // Pointer equality.
  return lhs.ptr() == rhs.ptr();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Inequality of flat_set
/// @related flat_set
///
/// O(1).
template <typename Value>
inline
bool
operator!=(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  // Pointer inequality.
  return not(lhs.ptr() == rhs.ptr());
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Comparison of flat_set
/// @related flat_set
///
/// O(1). The order on flat_set is arbitrary. 
template <typename Value>
inline
bool
operator<(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  // Pointer comparison.
  return lhs.ptr() < rhs.ptr();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Textual output of a flat_set
/// @related flat_set
template <typename Value>
std::ostream&
operator<<(std::ostream& os, const flat_set<Value>& fs)
{
  os << "{";
  if (not fs.empty())
  {
    std::copy(fs.cbegin(), std::prev(fs.cend()), std::ostream_iterator<Value>(os, ","));
    os << *std::prev(fs.cend());
  }
  return os << "}";
}

/*------------------------------------------------------------------------------------------------*/

/// @related flat_set
template <typename Value>
inline
flat_set<Value>
difference(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  typename flat_set<Value>::data_type res;
  std::set_difference( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                     , std::inserter(res, res.begin()));
  return {std::move(res)};
}

/*------------------------------------------------------------------------------------------------*/

/// @related flat_set
template <typename Value>
inline
flat_set<Value>
intersection(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  typename flat_set<Value>::data_type res;
  std::set_intersection( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                       , std::inserter(res, res.begin()));
  return {std::move(res)};
}

/*------------------------------------------------------------------------------------------------*/

/// @related flat_set
template <typename Value>
inline
flat_set<Value>
sum(const flat_set<Value>& lhs, const flat_set<Value>& rhs)
noexcept
{
  typename flat_set<Value>::data_type res;
  std::set_union( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                , std::inserter(res, res.begin()));
  return {std::move(res)};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::values::flat_set
template <typename Value>
struct hash<sdd::values::flat_set<Value>>
{
  std::size_t
  operator()(const sdd::values::flat_set<Value>& fs)
  const noexcept
  {
    return sdd::util::hash(fs.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_VALUES_flat_set_HH_
