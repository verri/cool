// Enumeration map implementation.

#ifndef COOL_ENUM_MAP_HXX_INCLUDED
/// \exclude
#define COOL_ENUM_MAP_HXX_INCLUDED

#if __cplusplus < 201703L
#error "cool::enum_map requires C++17"
#else

#include <array>
#include <type_traits>

namespace cool
{

/// \exclude
namespace detail
{
template <typename, auto> struct enum_value_t;

template <auto V> struct enum_key_t
{
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

template <typename T, auto V, auto... Vs> class enum_map
{
public:
  using index_type = decltype(V);
  static constexpr auto order = sizeof...(Vs) + 1u;

  static_assert(std::is_enum_v<index_type>);
  static_assert(std::conjunction_v<std::is_same<index_type, decltype(Vs)>...>);

  constexpr enum_map() = default;

  // Compile-time construction and access.
  template <typename W, typename... Ws> constexpr enum_map(W w, Ws... ws) : values{std::move(w.value), std::move(ws.value)...}
  {
    static_assert(std::is_same_v<detail::enum_value_t<T, V>, W>);
    static_assert(std::conjunction_v<std::is_same<detail::enum_value_t<T, Vs>, Ws>...>);
  }

  template <auto W> constexpr auto at(detail::enum_key_t<W>) noexcept -> T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto at(detail::enum_key_t<W>) const noexcept -> const T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto operator[](detail::enum_key_t<W>) noexcept -> T& { return values[to_index<W>()]; }

  template <auto W> constexpr auto operator[](detail::enum_key_t<W>) const noexcept -> const T& { return values[to_index<W>()]; }

  // Runtime construction and access.
  constexpr enum_map(std::initializer_list<std::pair<index_type, T>> values)
  {
    for (const auto& [i, value] : values) {
      this->values.at(to_index(i)) = value;
    }
  }

  constexpr auto at(index_type i) -> T& { return values.at(to_index(i)); }

  constexpr auto at(index_type i) const -> const T& { return values.at(to_index(i)); }

  constexpr auto operator[](index_type i) noexcept -> T& { return values[to_index(i)]; }

  constexpr auto operator[](index_type i) const noexcept -> const T& { return values[to_index(i)]; }

private:
  std::array<T, order> values;

  static constexpr auto to_index(index_type W) -> std::size_t
  {
    if (W == V)
      return 0;

    std::size_t i = 1u;
    return ((W == Vs ? true : (++i, false)) || ...), i;
  }

  template <index_type W> static constexpr auto to_index() -> std::size_t
  {
    static_assert(to_index(W) < order);
    return to_index(W);
  }

  template <index_type W> static constexpr std::size_t index = to_index<W>();
};

} // namespace cool

#endif // C++17
#endif // COOL_ENUM_MAP_HXX_INCLUDED
