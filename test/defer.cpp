#include <cool/defer.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic defer functionality", "[defer]")
{
  // COOL_DEFER guarantees that a statement or block is run after the current scope ends.

  int a = 0;

  // At the end of the scope, "a" will become "1".
  COOL_DEFER(a = 1);

  // "a" still equals to 0, because the scope has not ended yet.
  CHECK(a == 0);

  {
    COOL_DEFER(a = 2);
    CHECK(a == 0);
  } // Deferred "a = 2" is executed here.
  CHECK(a == 2);

  int x = 0;
  {
    // Deferred block is permitted as well.
    COOL_DEFER({
      for (int i = 0; i < 10; ++i)
        x += i;
    });
    CHECK(x == 0);
  }
  CHECK(x == 45);

  // Deferred code runs even if a exception is thrown.
  try {
    COOL_DEFER(x = 0);
    throw std::runtime_error{""};
  } catch (...) {
  }
  CHECK(x == 0);
}
