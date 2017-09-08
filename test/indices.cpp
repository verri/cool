#include <cool/indices.hpp>

#include <catch.hpp>

#include <algorithm>

TEST_CASE("Indices range", "[indices]")
{
  const auto a = cool::indices(10);
  const auto b = cool::indices(0ul, 10ul);

  CHECK(a.size() == 10);
  CHECK(b.size() == 10);

  CHECK(std::equal(a.begin(), a.end(), b.begin()));

  {
    typename decltype(a)::value_type i = 0;
    for (const auto j : a)
      CHECK(j == i++);
  }
  {
    typename decltype(a)::value_type i = 0;
    for (const auto j : b)
      CHECK(j == i++);
  }
}
