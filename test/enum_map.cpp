#include "catch.hpp"

#include <cool/enum_map.hpp>

TEST_CASE("Compile-time enum map operations", "[enum_map]")
{
  enum { A, B, C };
  using namespace cool;

  constexpr enum_map<int, A, B, C> map(key<A>(1), key<B>(2), key<C>(3));
  static_assert(map[key<A>] == 1);
  static_assert(map[key<B>] == 2);
  static_assert(map[key<C>] == 3);
}

TEST_CASE("Runtime enum map operations", "[enum_map]")
{
  enum { A, B, C, D };
  using namespace cool;

  enum_map<int, A, B, C> map{{A, 1}, {B, 2}, {C, 3}};
  CHECK(map[A] == 1);
  CHECK(map[B] == 2);
  CHECK(map[C] == 3);

  CHECK_THROWS(map.at(D));
}
