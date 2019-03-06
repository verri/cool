// Enumeration map implementation.

#ifndef COOL_ENUM_MAP_HXX_INCLUDED
/// \exclude
#define COOL_ENUM_MAP_HXX_INCLUDED

#if __cplusplus < 201703L
#error "cool::enum_map requires C++17"
#else

#include <array>
#include <iterator>
#include <type_traits>

namespace cool
{

/// \exclude
namespace detail
{
template <typename, auto> struct enum_value_t;

template <auto V> struct enum_key_t {
  static_assert(std::is_enum_v<decltype(V)>);

  template <typename T> constexpr auto operator()(T value) const -> enum_value_t<T, V>
  {
    return enum_value_t<T, V>(std::move(value));
  }
};

template <typename T, auto V> struct enum_value_t : enum_key_t<V> {
  constexpr explicit enum_value_t(T x) : value(std::move(x)) {}
  T value;
};
} // namespace detail

template <auto V> constexpr detail::enum_key_t<V> enum_key;

template <typename E, typename T> class enum_map_iterator
{
private:
  using key_type = const E;
  using mapped_type = T;

public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::pair<key_type, mapped_type&>;
  using difference_type = typename std::array<T, 1>::difference_type;
  using pointer = void;
  using reference = value_type;

  constexpr explicit enum_map_iterator() noexcept = default;

  constexpr enum_map_iterator(key_type* key_it, mapped_type* mapped_it) noexcept : key_it{key_it}, mapped_it{mapped_it} {}

  constexpr enum_map_iterator(const enum_map_iterator& source) noexcept = default;
  constexpr enum_map_iterator(enum_map_iterator&& source) noexcept = default;

  constexpr enum_map_iterator& operator=(const enum_map_iterator& source) noexcept = default;
  constexpr enum_map_iterator& operator=(enum_map_iterator&& source) noexcept = default;

  constexpr auto operator++() noexcept -> enum_map_iterator&
  {
    ++key_it;
    ++mapped_it;
    return *this;
  }

  constexpr auto operator++(int) noexcept -> enum_map_iterator
  {
    auto copy = *this;
    ++(*this);
    return copy;
  }

  constexpr auto operator--() noexcept -> enum_map_iterator&
  {
    --key_it;
    --mapped_it;
    return *this;
  }

  constexpr auto operator--(int) noexcept -> enum_map_iterator
  {
    auto copy = *this;
    --(*this);
    return copy;
  }

  constexpr auto operator+(difference_type n) const noexcept -> enum_map_iterator { return {key_it + n, mapped_it + n}; }

  constexpr auto operator-(difference_type n) const noexcept -> enum_map_iterator { return {key_it - n, mapped_it - n}; }

  constexpr auto operator==(const enum_map_iterator& other) const noexcept { return mapped_it == other.mapped_it; }

  constexpr auto operator!=(const enum_map_iterator& other) const noexcept { return mapped_it != other.mapped_it; }

  constexpr auto operator<(const enum_map_iterator& other) const noexcept { return mapped_it > other.mapped_it; }

  constexpr auto operator>(const enum_map_iterator& other) const noexcept { return mapped_it < other.mapped_it; }

  constexpr auto operator<=(const enum_map_iterator& other) const noexcept { return mapped_it <= other.mapped_it; }

  constexpr auto operator>=(const enum_map_iterator& other) const noexcept { return mapped_it >= other.mapped_it; }

  constexpr auto operator-(const enum_map_iterator& other) const noexcept { return std::distance(other.mapped_it, mapped_it); }

  constexpr auto operator*() noexcept -> value_type { return {*key_it, *mapped_it}; }

  constexpr auto operator[](difference_type n) const noexcept -> value_type { return {key_it[n], mapped_it[n]}; }

private:
  key_type* key_it;
  mapped_type* mapped_it;
};

template <typename T, auto V, auto... Vs> class enum_map
{
public:
  using key_type = std::decay_t<decltype(V)>;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;

  static constexpr auto order = sizeof...(Vs) + 1u;

  using storage_type = std::array<mapped_type, order>;
  using size_type = typename storage_type::size_type;
  using difference_type = typename storage_type::difference_type;

  using iterator = enum_map_iterator<key_type, mapped_type>;
  using const_iterator = enum_map_iterator<key_type, const mapped_type>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static constexpr std::array<key_type, order> keys = {V, Vs...};

  static_assert(std::is_enum_v<key_type>);
  static_assert(std::conjunction_v<std::is_same<key_type, decltype(Vs)>...>);

  constexpr enum_map() = default;

  // Compile-time construction and access.
  template <typename W, typename... Ws> constexpr enum_map(W w, Ws... ws) : values{std::move(w.value), std::move(ws.value)...}
  {
    static_assert(std::is_same_v<detail::enum_value_t<T, V>, W>);
    static_assert(std::conjunction_v<std::is_same<detail::enum_value_t<T, Vs>, Ws>...>);
  }

  constexpr explicit enum_map(std::array<T, order> values) : values(std::move(values)) {}

  template <auto W> constexpr auto at(detail::enum_key_t<W>) noexcept -> T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto at(detail::enum_key_t<W>) const noexcept -> const T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto operator[](detail::enum_key_t<W>) noexcept -> T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto operator[](detail::enum_key_t<W>) const noexcept -> const T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto find(detail::enum_key_t<W>) noexcept -> iterator { return begin() + to_index(W); }

  template <auto W> constexpr auto find(detail::enum_key_t<W>) const noexcept -> const_iterator { return begin() + to_index(W); }

  // Runtime construction and access.
  constexpr enum_map(std::initializer_list<value_type> values)
  {
    for (const auto& [i, value] : values) {
      this->values.at(to_index(i)) = value;
    }
  }

  template <typename InputIt> constexpr enum_map(InputIt begin, InputIt end)
  {
    static_assert(std::is_same_v<key_type, typename std::iterator_traits<InputIt>::value_type::first_type>);
    static_assert(std::is_assignable_v<mapped_type, typename std::iterator_traits<InputIt>::value_type::second_type>);

    for (; begin != end; ++begin)
      values.at(to_index(begin.first)) = begin.second;
  }

  constexpr auto at(key_type i) -> T& { return values.at(to_index(i)); }

  constexpr auto at(key_type i) const -> const T& { return values.at(to_index(i)); }

  constexpr auto operator[](key_type i) noexcept -> T& { return values[to_index(i)]; }

  constexpr auto operator[](key_type i) const noexcept -> const T& { return values[to_index(i)]; }

  constexpr auto find(key_type i) -> iterator { return begin() + to_index(i); }

  constexpr auto find(key_type i) const -> const_iterator { return begin() + to_index(i); }

  // Capacity
  constexpr auto empty() const noexcept -> bool { return false; }
  constexpr auto size() const noexcept -> size_type { return order; }
  constexpr auto max_size() const noexcept -> size_type { return order; }

  constexpr auto data() -> T* { return values.data(); }
  constexpr auto data() const -> const T* { return values.data(); }

  // Iterators
  constexpr auto begin() noexcept -> iterator { return {keys.data(), data()}; }
  constexpr auto begin() const noexcept -> const_iterator { return {keys.data(), data()}; }
  constexpr auto cbegin() const noexcept -> const_iterator { return {keys.data(), data()}; }

  constexpr auto end() noexcept -> iterator { return {keys.data() + order, data() + order}; }
  constexpr auto end() const noexcept -> const_iterator { return {keys.data() + order, data() + order}; }
  constexpr auto cend() const noexcept -> const_iterator { return {keys.data() + order, data() + order}; }

  constexpr auto rbegin() noexcept -> reverse_iterator { return std::make_reverse_iterator(end()); }
  constexpr auto rbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(end()); }
  constexpr auto crbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cend()); }

  constexpr auto rend() noexcept -> reverse_iterator { return std::make_reverse_iterator(begin()); }
  constexpr auto rend() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(begin()); }
  constexpr auto crend() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cbegin()); }

private:
  std::array<T, order> values;

  static constexpr auto to_index(key_type W) noexcept -> size_type
  {
    if (W == V)
      return 0;

    size_type i = 1u;
    return (void)((W == Vs ? true : (++i, false)) || ...), i;
  }

  template <key_type W> static constexpr auto to_index() noexcept -> size_type
  {
    static_assert(to_index(W) < order);
    return to_index(W);
  }

  template <key_type W> static constexpr size_type index = to_index<W>();
};

} // namespace cool

#endif // C++17
#endif // COOL_ENUM_MAP_HXX_INCLUDED
