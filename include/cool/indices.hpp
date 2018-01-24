#ifndef COOL_INDICES_HPP_INCLUDED
#define COOL_INDICES_HPP_INCLUDED

#include <iterator>
#include <type_traits>

#if __cpp_constexpr >= 201304
#define RELAXED_CONSTEXPR constexpr
#else
#define RELAXED_CONSTEXPR
#endif

namespace cool
{

template <typename T> class index_iterator
{
  static_assert(std::is_integral<T>::value, "T must be integral");

public:
  using value_type = T;
  using reference = T&;
  using pointer = T*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

  constexpr index_iterator() noexcept = default;

  constexpr explicit index_iterator(T value) noexcept : value_{std::move(value)} {}

  RELAXED_CONSTEXPR auto operator++() noexcept -> index_iterator&
  {
    ++value_;
    return *this;
  }

  RELAXED_CONSTEXPR auto operator++(int)noexcept -> index_iterator
  {
    auto copy = *this;
    ++(*this);
    return copy;
  }

  RELAXED_CONSTEXPR auto operator--() noexcept -> index_iterator&
  {
    --value_;
    return *this;
  }

  RELAXED_CONSTEXPR auto operator--(int)noexcept -> index_iterator
  {
    auto copy = *this;
    --(*this);
    return copy;
  }

  RELAXED_CONSTEXPR auto operator*() noexcept -> reference { return value_; }

  constexpr auto operator-(const index_iterator& other) const noexcept -> difference_type { return value_ - other.value_; }

  constexpr auto operator==(const index_iterator& other) const noexcept -> bool { return value_ == other.value_; }

  constexpr auto operator!=(const index_iterator& other) const noexcept -> bool { return value_ != other.value_; }

private:
  T value_;
};

template <typename T> class index_range
{
public:
  using value_type = T;
  using size_type = std::size_t;

  constexpr index_range(T begin, T end) noexcept : begin_{std::move(begin)}, end_{std::move(end)} {}

  constexpr auto begin() const noexcept -> index_iterator<T> { return begin_; }
  constexpr auto end() const noexcept -> index_iterator<T> { return end_; }

  constexpr auto size() const noexcept -> size_type { return end_ - begin_; }

private:
  index_iterator<T> begin_, end_;
};

template <typename T>
constexpr auto indices(T value) noexcept -> typename std::enable_if<std::is_integral<T>::value, index_range<T>>::type
{
  return {T{}, std::move(value)};
}

template <typename T, typename U>
constexpr auto indices(T begin, U end) noexcept ->
  typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value,
                          index_range<typename std::common_type<T, U>::type>>::type
{
  return {std::move(begin), std::move(end)};
}

template <typename T, typename U>
RELAXED_CONSTEXPR auto closed_indices(T begin, U end) noexcept ->
  typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value,
                          index_range<typename std::common_type<T, U>::type>>::type
{
  return {std::move(begin), std::move(++end)};
}

} // namespace cool

#undef RELAXED_CONSTEXPR

#endif // COOL_INDICES_HPP_INCLUDED
