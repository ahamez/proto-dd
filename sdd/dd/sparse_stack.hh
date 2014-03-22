#ifndef _SDD_DD_SPARSE_STACK_HH_
#define _SDD_DD_SPARSE_STACK_HH_

#include <algorithm> // copy
#include <cstdint>   // uint32_t
#include <deque>
#include <limits>    // numeric_limits
#include <numeric>   // accumulate

#include <boost/iterator/iterator_facade.hpp>

#include "sdd/dd/sparse_value.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename ContainerIterator>
class sparse_stack_iterator
  : public boost::iterator_facade< sparse_stack_iterator<T, ContainerIterator>
                                 , const typename T::value_type, boost::forward_traversal_tag>
{
private:

  using value_type = typename T::value_type;

  /// @brief The position in the sparse_stack's underlying container.
  ContainerIterator cit_;

  /// @brief The end of the sparse_stack's underlying container.
  ContainerIterator end_;

  /// @brief Count the number of remaining values (real or default).
  std::size_t remaining_;

  /// @brief Count the number of default values to skip for the current entry.
  std::uint32_t to_skip_;

  /// @brief Keep the default value as a member to return it as a reference.
  const value_type default_value;

public:

  /// @brief Default constructor.
  sparse_stack_iterator() = default;

  /// @brief Constructor.
  explicit sparse_stack_iterator(ContainerIterator cit, ContainerIterator end, std::size_t max)
  noexcept
    : cit_(cit), end_(end), remaining_(max)
      // cit can be equal to end when a sparse_stack contains only default values
    , to_skip_(cit == end ? std::numeric_limits<std::uint32_t>::max() : cit->skip)
    , default_value(T::default_value())
  {}

  /// @brief Copy constructor
  sparse_stack_iterator(const sparse_stack_iterator& other)
  noexcept
    : cit_(other.cit_), end_(other.end_), remaining_(other.remaining_), to_skip_(other.to_skip_)
    , default_value(T::default_value())
  {}

  /// @brief Copy operator
  sparse_stack_iterator&
  operator=(const sparse_stack_iterator& other)
  {
    cit_ = other.cit_;
    end_ = other.end_;
    remaining_ = other.remaining_;
    to_skip_ = other.to_skip_;
    return *this;
  }

private:

  // Required by boost::iterator.
  friend class boost::iterator_core_access;

  /// @brief For boost::iterator.
  void
  increment()
  noexcept
  {
    --remaining_;
    if (cit_ != end_)
    {
      if (to_skip_ > 0) // There are still some default values for the current entry.
      {
        --to_skip_;
      }
      else // No more default values for the current entry.
      {
        ++cit_;
        to_skip_ = cit_ == end_ ? std::numeric_limits<std::uint32_t>::max() : cit_->skip;
      }
    }
    else // Past the end of the sparse_stack's underlying container.
    {
      to_skip_ = std::numeric_limits<std::uint32_t>::max();
    }
  }

  /// @brief For boost::iterator.
  bool
  equal(const sparse_stack_iterator& other)
  const noexcept
  {
    return remaining_ == other.remaining_;
  }

  /// @brief For boost::iterator.
  const value_type&
  dereference()
  const noexcept
  {
    return to_skip_ > 0 ? default_value : cit_->value;
  }
}; // class sparse_stack_iterator

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename ContainerIterator>
class sparse_stack_reverse_iterator
  : public boost::iterator_facade< sparse_stack_reverse_iterator<T, ContainerIterator>
                                 , const typename T::value_type, boost::forward_traversal_tag>
{
private:

  using value_type = typename T::value_type;

  /// @brief The position in the sparse_stack's underlying container.
  ContainerIterator cit_;

  /// @brief The end of the sparse_stack's underlying container.
  ContainerIterator end_;


  std::uint32_t initial_skip_;
  std::uint32_t actual_skip_;

  /// @brief Count the number of default values to skip for the current entry.
  std::uint32_t to_skip_;

  /// @brief Keep the default value as a member to return it as a reference.
  const value_type default_value;

public:

  /// @brief Default constructor.
  sparse_stack_reverse_iterator() = default;

  /// @brief Constructor.
  explicit sparse_stack_reverse_iterator( ContainerIterator cit, ContainerIterator end
                                        , std::size_t limit, std::size_t size)
  noexcept
    : cit_(cit), end_(end)
    , initial_skip_(cit == end ? 0 : static_cast<std::uint32_t>(limit - size))
    , actual_skip_(0)
    , to_skip_(0)
    , default_value(T::default_value())
  {
    assert(limit >= size);
  }

  /// @brief Copy constructor
  sparse_stack_reverse_iterator(const sparse_stack_reverse_iterator& other)
  noexcept
    : cit_(other.cit_), end_(other.end_), initial_skip_(other.initial_skip_)
    , actual_skip_(other.actual_skip_), to_skip_(other.to_skip_)
    , default_value(T::default_value())
  {}

  /// @brief Copy operator
  sparse_stack_reverse_iterator&
  operator=(const sparse_stack_reverse_iterator& other)
  {
    cit_ = other.cit_;
    end_ = other.end_;
    initial_skip_ = other.initial_skip_;
    actual_skip_ = other.actual_skip_;
    to_skip_ = other.to_skip_;
    return *this;
  }

private:

  // Required by boost::iterator.
  friend class boost::iterator_core_access;

  /// @brief For boost::iterator.
  void
  increment()
  noexcept
  {
    if (initial_skip_ > 0)
    {
      initial_skip_ -= 1;
      if (initial_skip_ == 0)
      {
        actual_skip_ = cit_->skip;
        to_skip_ = cit_->skip;
      }
    }
    else
    {
      if (to_skip_ == 0)
      {
        ++cit_;
        if (cit_ == end_)
        {
          actual_skip_ = 0;
        }
        else
        {
          actual_skip_ = cit_->skip;
          to_skip_ = cit_->skip;
        }
      }
      else
      {
        to_skip_ -= 1;
      }
    }
  }

  /// @brief For boost::iterator.
  bool
  equal(const sparse_stack_reverse_iterator& other)
  const noexcept
  {
    return initial_skip_ == other.initial_skip_ and actual_skip_ == other.actual_skip_
       and to_skip_ == other.to_skip_ and cit_ == other.cit_;
  }

  /// @brief For boost::iterator.
  const value_type&
  dereference()
  const noexcept
  {
    if (initial_skip_ > 0)
    {
      return default_value;
    }
    else
    {
      if (to_skip_ == actual_skip_)
      {
        return cit_->value;
      }
      else
      {
        return default_value;
      }
    }
  }
}; // class sparse_stack_iterator

/*------------------------------------------------------------------------------------------------*/

// Forward declaration.
/// @internal
template <typename T>
class sparse_stack;

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Act as proxy for a sparse_stack, with the upper bound fixed.
template <typename T>
class bounded_sparse_stack
{
private:

  using container_type = typename sparse_stack<T>::container_type;

  /// @brief The sparse stack's underlying container.
  const container_type& values_;

  /// @brief
  const std::size_t limit_;

  const std::size_t size_;

public:

  /// @brief The type of the iterator on a bounded sparse stack.
  using const_iterator = sparse_stack_iterator< T
                                              , typename container_type::const_reverse_iterator>;

  /// @brief The type of the iterator on a bounded sparse stack.
  using const_reverse_iterator = sparse_stack_reverse_iterator< T
                                              , typename container_type::const_iterator>;

public:

  /// @brief Constructor.
  bounded_sparse_stack(const container_type& values, std::size_t limit, std::size_t size)
    : values_(values), limit_(limit), size_(size)
  {}

  /// @brief Get the begin of the bounded stack.
  ///
  /// The beginning of the stack is its top element (most recently pushed).
  const_iterator
  begin()
  const noexcept
  {
    return const_iterator(values_.rbegin(), values_.rend(), limit_);
  }

  /// @brief Get the end of the bounded stack.
  ///
  /// The end of the stack is its last element (last recently pushed).
  const_iterator
  end()
  const noexcept
  {
    return const_iterator(values_.rbegin(), values_.rend(), 0);
  }

  /// @brief Get the begin of the bounded stack.
  ///
  /// The beginning of the stack is its top element (most recently pushed).
  const_reverse_iterator
  rbegin()
  const noexcept
  {
    return const_reverse_iterator(values_.begin(), values_.end(), limit_, size_);
  }

  /// @brief Get the end of the bounded stack.
  ///
  /// The end of the stack is its last element (last recently pushed).
  const_reverse_iterator
  rend()
  const noexcept
  {
    return const_reverse_iterator(values_.end(), values_.end(), 0, 0);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Value>
struct sparse_stack_entry
{
  /// @brief How many default values were skipped.
  std::uint32_t skip;

  /// @brief The value, different from the default value.
  const Value value;

  /// @brief Constructor.
  sparse_stack_entry(std::uint32_t s, const Value& v)
    : skip(s), value(v)
  {}
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
class sparse_stack
{
public:

  using value_type = typename T::value_type;

  /// @brief The container of entries.
  using container_type = std::deque<sparse_stack_entry<value_type>>;

  using bounded_stack = bounded_sparse_stack<T>;

private:

  /// @brief The storage of values with their associated skip value.
  container_type values_;
  
  /// @brief Store the virtual size to avoid computing it each time it's needed..
  std::size_t size_;

  /// @brief Construct from an already existing container.
  explicit sparse_stack( typename container_type::const_iterator begin
                       , typename container_type::const_iterator end
                       , std::size_t size)
    : values_(begin, end), size_(size)
  {}

  /// @brief Construct from an already existing container.
  explicit sparse_stack(container_type&& values, std::size_t size)
    : values_(std::move(values)), size_(size)
  {}

public:
  
  /// @brief Default constructor
  sparse_stack()
    : values_(), size_(0)
  {}

  /// @brief Copy constructor.
  sparse_stack(const sparse_stack&) = default;

  /// @brief Copy operator.
  sparse_stack& operator=(const sparse_stack&) = default;

  /// @brief Move constructor.
  sparse_stack(sparse_stack&&) = default;

  /// @brief Move operator.
  sparse_stack& operator=(sparse_stack&&) = default;

  /// @brief Tell if the stack doesn't contain any element.
  bool
  empty()
  const noexcept
  {
    return values_.empty();
  }

  /// @brief Push a value on the top of the stack.
  void
  push(const value_type& value)
  {
    if (value == T::default_value())
    {
      if (not empty())
      {
        // Add a default value on top of the stack.
        values_.back().skip += 1;
        ++size_;
      }
    }
    else
    {
      values_.emplace_back(0, value);
      ++size_;
    }
  }

  /// @brief Return a copy with the most recent value removed.
  sparse_stack
  pop()
  const
  {
    if (not empty())
    {
      if (values_.back().skip == 0) // not a default value
      {
        return sparse_stack(values_.cbegin(), std::prev(values_.cend()), size_ - 1);
      }
      else // a default value
      {
        container_type tmp(values_);
        tmp.back().skip -= 1;
        return sparse_stack(std::move(tmp), size_ - 1);
      }
    }
    else
    {
      return sparse_stack();
    }
  }

  /// @brief Get the size of the stack (real and default values).
  std::size_t
  size()
  const noexcept
  {
    return size_;
  }

  /// @brief Return a sparse stack with an upper bound.
  bounded_sparse_stack<T>
  limit(std::size_t lim)
  const noexcept
  {
    return bounded_sparse_stack<T>(values_, lim, size_);
  }

  /// @brief Access the real values
  const container_type&
  values()
  const noexcept
  {
    return values_;
  }

private:
  
  friend class bounded_sparse_stack<T>;
}; // class sparse_stack;

/*------------------------------------------------------------------------------------------------*/

/// @brief Equality
template <typename Value>
inline
bool
operator==(const sparse_stack_entry<Value>& lhs, const sparse_stack_entry<Value>& rhs)
noexcept
{
  return lhs.skip == rhs.skip and lhs.value == rhs.value;
}

/// @internal
/// @brief   Equality of two sparse_stack.
/// @related sparse_stack
template <typename T>
inline
bool
operator==(const sparse_stack<T>& lhs, const sparse_stack<T>& rhs)
noexcept
{
  return lhs.values() == rhs.values();
}

/// @brief   Export a sparse_stack to a stream.
/// @related sparse_stack
template <typename T>
std::ostream&
operator<<(std::ostream& os, const sparse_stack<T>& s)
{
  using value_type = typename sparse_stack<T>::value_type;
  const auto proxy = s.limit(s.size());
  std::copy(proxy.begin(), proxy.end(), std::ostream_iterator<value_type>(os, "\n"));
  return os;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::sparse_stack_entry
template <typename Value>
struct hash<sdd::dd::sparse_stack_entry<Value>>
{
  std::size_t
  operator()(const sdd::dd::sparse_stack_entry<Value>& e)
  const noexcept
  {
    std::size_t seed = sdd::util::hash(e.skip);
    sdd::util::hash_combine(seed, e.value);
    return seed;
  }
};

/// @brief Hash specialization for sdd::dd::sparse_stack
template <typename T>
struct hash<sdd::dd::sparse_stack<T>>
{
  std::size_t
  operator()(const sdd::dd::sparse_stack<T>& s)
  const
  {
    return sdd::util::hash(s.values().cbegin(), s.values().cend());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_SPARSE_STACK_HH_
