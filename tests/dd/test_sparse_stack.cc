#include "gtest/gtest.h"

#include "sdd/dd/sparse_stack.hh"

/*------------------------------------------------------------------------------------------------*/

struct uint_max
{
  using value_type = unsigned int;
  static constexpr unsigned int default_value() noexcept {return 0;}
};

template <typename T>
struct sparse_stack_test
  : public testing::Test
{
  using sparse_stack = sdd::dd::sparse_stack<T>;
};

using configurations = ::testing::Types<uint_max>;

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST_CASE(sparse_stack_test, configurations);
#define sparse_stack typename TestFixture::sparse_stack

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, empty)
{
  sparse_stack s;
  ASSERT_EQ(0, s.size());
  auto proxy = s.limit(10);
  unsigned int i = 0;
  for (const auto& v : proxy)
  {
    ++i;
    ASSERT_EQ(0, v);
  }
  ASSERT_EQ(10, i);
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, push_all_default)
{
  sparse_stack s;
  for (auto i = 0; i < 4; ++i)
  {
    s.push(0);
  }
  ASSERT_EQ(0, s.size());
  auto proxy = s.limit(10);
  unsigned int i = 0;
  for (const auto& v : proxy)
  {
    ++i;
    ASSERT_EQ(0, v);
  }
  ASSERT_EQ(10, i);
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, pop_all_default)
{
  sparse_stack s0;
  for (auto i = 0; i < 4; ++i)
  {
    s0.push(0);
  }
  ASSERT_EQ(0, s0.size());
  const auto s1 = s0.pop();
  ASSERT_EQ(0, s1.size());
  auto proxy = s1.limit(10);
  unsigned int i = 0;
  for (const auto& v : proxy)
  {
    ++i;
    ASSERT_EQ(0, v);
  }
  ASSERT_EQ(10, i);
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, push_normal)
{
  sparse_stack s;
  s.push(0);
  s.push(0);
  s.push(2);
  s.push(3);
  s.push(0);
  s.push(4);
  s.push(4);
  s.push(0);
  s.push(0);
  s.push(0);

  ASSERT_EQ(8, s.size());

  auto cit = s.limit(8).begin();
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(3, *cit++);
  ASSERT_EQ(2, *cit++);
  ASSERT_EQ(s.limit(8).end(), cit);

  cit = s.limit(5).begin();
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(s.limit(5).end(), cit);

  cit = s.limit(11).begin();
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(3, *cit++);
  ASSERT_EQ(2, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(s.limit(11).end(), cit);
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, reverse_iterator)
{
  {
    sparse_stack s;
    ASSERT_EQ(0, s.size());
    std::size_t i = 0;
    for (const auto& v : s.limit(10))
    {
      ASSERT_EQ(0, v);
      ++i;
    }
    ASSERT_EQ(10, i);
  }
  {
    sparse_stack s;
    s.push(0);
    s.push(0);
    s.push(2);
    s.push(3);
    s.push(0);
    s.push(4);
    s.push(4);
    s.push(0);
    s.push(0);
    s.push(0);

    ASSERT_EQ(8, s.size());

    auto cit = s.limit(8).rbegin();
    ASSERT_EQ(2, *cit++);
    ASSERT_EQ(3, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(4, *cit++);
    ASSERT_EQ(4, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(s.limit(8).rend(), cit);


    cit = s.limit(10).rbegin();
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(2, *cit++);
    ASSERT_EQ(3, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(4, *cit++);
    ASSERT_EQ(4, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(0, *cit++);
    ASSERT_EQ(s.limit(10).rend(), cit);
  }
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, pop_normal)
{
  sparse_stack s;
  s.push(0);
  s.push(0);
  s.push(2);
  s.push(3);
  s.push(0);
  s.push(4);
  s.push(4);
  s.push(0);
  s.push(0);
  ASSERT_EQ(7, s.size());

  const auto s1 = s.pop();
  ASSERT_EQ(6, s1.size());
  auto cit = s1.limit(6).begin();
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(3, *cit++);
  ASSERT_EQ(2, *cit++);
  ASSERT_EQ(s1.limit(6).end(), cit);

  const auto s2 = s1.pop();
  ASSERT_EQ(5, s2.size());
  cit = s2.limit(5).begin();
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(3, *cit++);
  ASSERT_EQ(2, *cit++);
  ASSERT_EQ(s2.limit(5).end(), cit);

  const auto s3 = s2.pop();
  ASSERT_EQ(4, s3.size());
  cit = s3.limit(4).begin();
  ASSERT_EQ(4, *cit++);
  ASSERT_EQ(0, *cit++);
  ASSERT_EQ(3, *cit++);
  ASSERT_EQ(2, *cit++);
  ASSERT_EQ(s3.limit(4).end(), cit);

  const auto s4 = s3.pop().pop().pop().pop();
  ASSERT_EQ(0, s4.size());
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, equality)
{
  {
    sparse_stack s0;
    s0.push(0);
    s0.push(0);
    s0.push(2);
    s0.push(3);
    s0.push(0);
    s0.push(4);
    s0.push(4);
    s0.push(0);
    s0.push(0);
    s0.push(0);

    sparse_stack s1;
    s1.push(0);
    s1.push(0);
    s1.push(2);
    s1.push(3);
    s1.push(0);
    s1.push(4);
    s1.push(4);
    s1.push(0);
    s1.push(0);
    s1.push(0);

    ASSERT_TRUE(s0 == s1);
  }
}

/*------------------------------------------------------------------------------------------------*/
TYPED_TEST(sparse_stack_test, hash)
{
  {
    sparse_stack s0;
    s0.push(0);
    s0.push(0);
    s0.push(2);
    s0.push(3);
    s0.push(0);
    s0.push(4);
    s0.push(4);
    s0.push(0);
    s0.push(0);
    s0.push(0);

    sparse_stack s1;
    s1.push(0);
    s1.push(0);
    s1.push(2);
    s1.push(3);
    s1.push(0);
    s1.push(4);
    s1.push(4);
    s1.push(0);
    s1.push(0);
    s1.push(0);

    ASSERT_TRUE(std::hash<sparse_stack>()(s0) == std::hash<sparse_stack>()(s1));
  }
  {
    sparse_stack s0;
    s0.push(0);
    s0.push(0);
    s0.push(2);
    s0.push(3);
    s0.push(0);
    s0.push(4);
    s0.push(4);
    s0.push(0);

    sparse_stack s1;
    s1.push(0);
    s1.push(2);
    s1.push(3);
    s1.push(0);
    s1.push(4);
    s1.push(4);
    s1.push(0);

    ASSERT_TRUE(std::hash<sparse_stack>()(s0) == std::hash<sparse_stack>()(s1));
  }
  {
    sparse_stack s0;
    s0.push(0);
    s0.push(0);
    s0.push(2);
    s0.push(3);
    s0.push(0);
    s0.push(4);
    s0.push(4);
    s0.push(0);

    sparse_stack s1;
    s1.push(0);
    s1.push(0);
    s1.push(2);
    s1.push(3);
    s1.push(0);
    s1.push(4);
    s1.push(4);

    ASSERT_FALSE(std::hash<sparse_stack>()(s0) == std::hash<sparse_stack>()(s1));
  }
}

/*------------------------------------------------------------------------------------------------*/
