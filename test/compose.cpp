#include <cool/compose.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cool;

TEST_CASE("Basic compose functionality", "[compose]")
{
  const auto f = compose([](int) { return 1; }, [](double) { return 2; });
  CHECK(f(1) == 1);
  CHECK(f(1.0) == 2);
}
