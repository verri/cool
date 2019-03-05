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

template <auto V> constexpr detail::enum_key_t<V> key;

template <typename T, auto V, auto... Vs> class enum_map_iterator;

template <typename T, auto V, auto... Vs> class enum_map
{
public:
  using key_type = decltype(V);
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;

  static constexpr auto order = sizeof...(Vs) + 1u;

  using storage_type = std::array<mapped_type, order>;
  using size_type = typename storage_type::size_type;
  using difference_type = typename storage_type::difference_type;

  using iterator = enum_map_iterator<T, V, Vs...>;
  using const_iterator = enum_map_iterator<const T, V, Vs...>;
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

  template <auto W> constexpr auto find(detail::enum_key_t<W>) noexcept -> iterator;

  template <auto W> constexpr auto find(detail::enum_key_t<W>) const noexcept -> const_iterator;

  // Runtime construction and access.
  constexpr enum_map(std::initializer_list<value_type> values)
  {
    for (const auto& [i, value] : values) {
      this->values.at(to_index(i)) = value;
    }
  }

  template <typename InputIt> constexpr enum_map(InputIt begin, InputIt end)
  {
    static_assert(std::is_same_v<key_type, typename InputIt::value_type::first_type>);
    static_assert(std::is_assignable_v<mapped_type, typename InputIt::value_type::second_type>);

    for (; begin != end; ++begin)
      values.at(to_index(begin.first)) = begin.second;
  }

  constexpr auto at(key_type i) -> T& { return values.at(to_index(i)); }

  constexpr auto at(key_type i) const -> const T& { return values.at(to_index(i)); }

  constexpr auto operator[](key_type i) noexcept -> T& { return values[to_index(i)]; }

  constexpr auto operator[](key_type i) const noexcept -> const T& { return values[to_index(i)]; }

  constexpr auto find(key_type i) -> iterator;
  constexpr auto find(key_type i) const -> const_iterator;

  // Capacity
  constexpr auto empty() const noexcept -> bool { return false; }
  constexpr auto size() const noexcept -> size_type { return order; }
  constexpr auto max_size() const noexcept -> size_type { return order; }

  // Iterators
  constexpr auto begin() noexcept -> iterator;
  constexpr auto cbegin() const noexcept -> const_iterator;

  constexpr auto end() noexcept -> iterator;
  constexpr auto cend() const noexcept -> const_iterator;

  constexpr auto rbegin() noexcept -> reverse_iterator;
  constexpr auto crbegin() const noexcept -> const_reverse_iterator;

  constexpr auto rend() noexcept -> reverse_iterator;
  constexpr auto crend() const noexcept -> const_reverse_iterator;

private:
  std::array<T, order> values;

  static constexpr auto to_index(key_type W) -> size_type
  {
    if (W == V)
      return 0;

    size_type i = 1u;
    return ((W == Vs ? true : (++i, false)) || ...), i;
  }

  template <key_type W> static constexpr auto to_index() -> size_type
  {
    static_assert(to_index(W) < order);
    return to_index(W);
  }

  template <key_type W> static constexpr size_type index = to_index<W>();
};

} // namespace cool

#endif // C++17
#endif // COOL_ENUM_MAP_HXX_INCLUDED
