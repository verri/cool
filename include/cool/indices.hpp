#ifndef COOL_INDICES_HPP_INCLUDED
#define COOL_INDICES_HPP_INCLUDED

#include <iterator>
#include <type_traits>

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

  index_iterator() = default;
  explicit index_iterator(T value) : value_{std::move(value)} {}

  auto operator++() -> index_iterator&
  {
    ++value_;
    return *this;
  }

  auto operator++(int) -> index_iterator
  {
    auto copy = *this;
    ++(*this);
    return copy;
  }

  auto operator--() -> index_iterator&
  {
    --value_;
    return *this;
  }

  auto operator--(int) -> index_iterator
  {
    auto copy = *this;
    --(*this);
    return copy;
  }

  auto operator*() -> reference { return value_; }

  auto operator-(const index_iterator& other) const -> difference_type { return value_ - other.value_; }

  auto operator==(const index_iterator& other) const -> bool { return value_ == other.value_; }
  auto operator!=(const index_iterator& other) const -> bool { return value_ != other.value_; }

private:
  T value_;
};

template <typename T> class index_range
{
public:
  using value_type = T;
  using size_type = std::size_t;

  index_range(T begin, T end) : begin_{std::move(begin)}, end_{std::move(end)} {}

  auto begin() const -> index_iterator<T> { return begin_; }
  auto end() const -> index_iterator<T> { return end_; }

  auto size() const -> size_type { return end_ - begin_; }

private:
  index_iterator<T> begin_, end_;
};

template <typename T> auto indices(T value) -> index_range<T> { return {T{}, std::move(value)}; }

template <typename T> auto indices(T begin, T end) -> index_range<T> { return {std::move(begin), std::move(end)}; }

} // namespace cool

#endif // COOL_INDICES_HPP_INCLUDED
