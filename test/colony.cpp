#include <cool/colony.hpp>

#include <catch.hpp>

#include <memory>
#include <random>
#include <set>
#include <utility>

using namespace cool;

TEST_CASE("Basic colony operations", "[colony]")
{
  std::set<int> set, erased;
  colony<int> c;

  std::mt19937 gen(11);
  std::bernoulli_distribution dist(0.5);

  for (int i = 0; i < 5; ++i) {
    for (int j = 10 * i; j < 10 * i + 10; ++j) {
      set.insert(j);
      c.push(j);
    }

    for (auto it = c.begin(); it != colony<int>::sentinel{};) {
      if (dist(gen)) {
        ++it;
      } else {
        erased.insert(*it);
        set.erase(*it);
        it = c.erase(it);
      }
    }

    CHECK(c.size() == set.size());
  }

  for (const auto i : static_cast<const colony<int>&>(c))
    CHECK(set.count(i) > 0);

  // NOTE: legacy end is needed here.
  for (const auto i : erased)
    CHECK(std::find(c.begin(), c.lend(), i) == c.lend());
}

TEST_CASE("Check removing all elements", "[colony]")
{
  colony<std::unique_ptr<int>> c;

  for (int i = 0; i < 5; ++i)
    c.emplace(new int{i});

  CHECK_FALSE(c.empty());

  for (auto it = c.begin(); it != c.end();)
    it = c.erase(it);

  CHECK(c.empty());
}
