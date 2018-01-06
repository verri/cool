#include <cool/defer.hpp>

#include <catch.hpp>

TEST_CASE("Basic defer functionality", "[defer]")
{
  int a = 0;

  COOL_DEFER(a = 1);
  CHECK(a == 0);

  {
    COOL_DEFER(a = 2);
    CHECK(a == 0);
  }

  CHECK(a == 2);
}
