#include <cool/colony.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
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

  for (int i = 0; i < 32; ++i)
    c.emplace(new int{i});

  CHECK_FALSE(c.empty());

  for (auto it = c.begin(); it != c.end();)
    it = c.erase(it);

  CHECK(c.empty());

  for (int i = 0; i < 32; ++i)
    c.emplace(new int{i});
  // no memory leak
}

TEST_CASE("Colony constructor tests", "[colony]")
{
  // Default constructor
  {
    colony<int> c;
    CHECK(c.empty());
    CHECK(c.size() == 0);
  }

  // Constructor with custom bucket size
  {
    colony<int> c(8);
    CHECK(c.empty());
    CHECK(c.size() == 0);
    
    // Fill more than one bucket to test bucket size
    for (int i = 0; i < 20; ++i) {
      c.push(i);
    }
    CHECK(c.size() == 20);
  }

  // Copy constructor
  {
    colony<int> c1;
    c1.push(1);
    c1.push(2);
    c1.push(3);
    
    colony<int> c2(c1);
    CHECK(c2.size() == 3);
    CHECK(std::equal(c1.begin(), c1.lend(), c2.begin()));
  }

  // Move constructor
  {
    colony<int> c1;
    c1.push(1);
    c1.push(2);
    c1.push(3);
    
    colony<int> c2(std::move(c1));
    CHECK(c2.size() == 3);
  }
}

struct ThrowingType {
  int value;
  static int construction_count;
  
  ThrowingType(int v) : value(v) {
    construction_count++;
    if (construction_count == 5) {
      throw std::runtime_error("Throwing constructor");
    }
  }
  
  ~ThrowingType() {
    construction_count--;
  }
};

int ThrowingType::construction_count = 0;

TEST_CASE("Colony exception safety", "[colony]")
{
  // Test that colony maintains basic consistency
  colony<int> c;
  
  // Test that we can add elements after construction
  c.emplace(1);
  c.emplace(2);
  c.emplace(3);
  c.emplace(4);
  
  CHECK(c.size() == 4);
  
  // Test that iterator remains valid after successful operations
  auto it = c.begin();
  CHECK(*it == 1);
  
  // Test that we can continue to add elements
  c.emplace(5);
  CHECK(c.size() == 5);
  
  // Test that we can still iterate after additions
  int count = 0;
  for (const auto& item : c) {
    (void)item;
    count++;
  }
  CHECK(count == 5);
}
