#include "catch2/catch_test_macros.hpp"

#include <cool/enum_map.hpp>

TEST_CASE("Compile-time enum map operations", "[enum_map]")
{
  enum { A, B, C, D };
  using namespace cool;

  static constexpr enum_map<int, A, B, D> map(enum_key<A>(1), enum_key<B>(2), enum_key<D>(3));
  static_assert(map[enum_key<A>] == 1);
  static_assert(map[enum_key<B>] == 2);
  static_assert(map[enum_key<D>] == 3);
  static_assert(map.find(enum_key<A>) == map.begin());
  static_assert(map.find(enum_key<B>) == map.begin() + 1);
  static_assert(map.find(enum_key<D>) == map.begin() + 2);
  static_assert(map.find(enum_key<C>) == map.end());
  static_assert(!map.empty());
  static_assert(map.size() == 3u);
  static_assert(map.max_size() == 3u);
}

TEST_CASE("Runtime enum map operations", "[enum_map]")
{
  enum { A, B, C, D };
  using namespace cool;

  enum_map<int, A, B, C> map{{A, 1}, {B, 2}, {C, 3}};
  CHECK(map[A] == 1);
  CHECK(map[B] == 2);
  CHECK(map[C] == 3);

  {
    const auto keys = {A, B, C};
    int i = 0;
    auto key_it = keys.begin();

    for (const auto& [key, value] : map) {
      CHECK(*key_it++ == key);
      CHECK(++i == value);
    }
  }

  for (auto [key, value] : map) {
    (void)key;
    value = 0;
  }

  {
    auto it = map.crbegin();
    while (it != map.crend())
      CHECK((*it++).second == 0);
  }

  CHECK(map.find(D) == map.end());
  CHECK_THROWS(map.at(D));
}
