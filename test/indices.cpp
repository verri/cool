#include <cool/indices.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

TEST_CASE("Basic indices functionality", "[indices]")
{
  // cool::indices is meant to increase safety in for loops.

  // for (int i = 0; i < 3; ++i)
  //          ^-- "i" cannot be const, programmer might change it.
  //   for (int j = 0; j < i; ++i)
  //                             ^-- bug here.

  // for (const auto i : cool::indices(3))
  //                 ^-- "i" is immutable.
  //   for (const auto j : cool::indices(i))

  for (const auto i : cool::indices(3)) {
    //            ^-- type of "i" is inferred.
    CHECK(0 <= i); // "i" ∈ [0, 3)
    CHECK(i < 3);  // "i" is immutable here.
  }

  for (const auto i : cool::indices(0, 3L)) {
    // "i" ∈ [0, 3)
    CHECK(0 <= i);
    CHECK(i < 3);

    // Type of "i" is the common type of the arguments.
    static_assert(std::is_same<typename std::remove_const<decltype(i)>::type,
                               typename std::common_type<decltype(0), decltype(3L)>::type>::value,
                  "that shouldn't happen");
  }

  std::vector<char> letters_vec;
  letters_vec.resize('z' - 'a' + 1);
  std::iota(letters_vec.begin(), letters_vec.end(), 'a');

  // Closed-range.
  const auto letters_range = cool::closed_indices('a', 'z');

  // Has the same size, but only stores the limits.
  CHECK(letters_vec.size() == letters_range.size());

  // Both iterate through the same values.
  const auto res = std::mismatch(letters_vec.begin(), letters_vec.end(), letters_range.begin());
  CHECK(res.first == letters_vec.end());
  CHECK(res.second == letters_range.end());
}

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
    typename decltype(b)::value_type i = 0;
    for (const auto j : b)
      CHECK(j == i++);
  }
}

#if __cpp_lib_integer_sequence >= 201304
TEST_CASE("Do indices", "[indices]")
{
  cool::do_indices<2>([](std::size_t i, std::size_t j) {
    CHECK(i == 0);
    CHECK(j == 1);
  });
}
#endif

#if __cplusplus >= 202000L
// NOTE: in C++20 can be a lambda
template <typename T, std::size_t N> auto array_sum(const std::array<T, N>& array)
{
  return cool::do_indices<N>([&array](auto... i) { return (T{} + ... + array[i]); });
}

TEST_CASE("Do indices to index array", "[indices]")
{
  CHECK(6 == array_sum(std::array{1, 2, 3}));
  CHECK(3 == array_sum(std::array{1, 2, 3, -3}));
}
#endif
