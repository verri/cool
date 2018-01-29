---
---

# Header file `indices.hpp`

``` cpp
namespace cool
{
    template <typename T>
    class index_iterator;

    template <typename T>
    class index_range;

    template <typename T>
    constexpr typename std::enable_if<std::is_integral<T>::value, index_range<T> >::type indices(T value) noexcept;

    template <typename T, typename U>
    constexpr typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value, index_range<typename std::common_type<T, U>::type> >::type indices(T begin, U end) noexcept;

    template <typename T, typename U>
    typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value, index_range<typename std::common_type<T, U>::type> >::type closed_indices(T begin, U end) noexcept;
}
```

### Class `cool::index_iterator` \[Indices\]

``` cpp
template <typename T>
class index_iterator
{
public:
    using value_type = T;

    using reference = T&;

    using pointer = T*;

    using difference_type = std::ptrdiff_t;

    using iterator_category = std::bidirectional_iterator_tag;

    constexpr index_iterator() noexcept = default;

    constexpr index_iterator(T value) noexcept;

    index_iterator<T>& operator++() noexcept;

    index_iterator<T> operator++(int) noexcept;

    index_iterator<T>& operator--() noexcept;

    index_iterator<T> operator--(int) noexcept;

    reference operator*() noexcept;

    constexpr difference_type operator-(const index_iterator<T>& other) const noexcept;

    constexpr bool operator==(const index_iterator<T>& other) const noexcept;

    constexpr bool operator!=(const index_iterator<T>& other) const noexcept;
};
```

-----

### Class `cool::index_range` \[Indices\]

``` cpp
template <typename T>
class index_range
{
public:
    using value_type = T;

    using size_type = std::size_t;

    constexpr index_range(T begin, T end) noexcept;

    constexpr index_iterator<T> begin() const noexcept;

    constexpr index_iterator<T> end() const noexcept;

    constexpr size_type size() const noexcept;
};
```

-----

### Function `cool::indices` \[Indices\]

``` cpp
template <typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, index_range<T> >::type indices(T value) noexcept;
```

-----

### Function `cool::indices` \[Indices\]

``` cpp
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value, index_range<typename std::common_type<T, U>::type> >::type indices(T begin, U end) noexcept;
```

-----

### Function `cool::closed_indices` \[Indices\]

``` cpp
template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value, index_range<typename std::common_type<T, U>::type> >::type closed_indices(T begin, U end) noexcept;
```

-----
