---
---

# Header file `colony.hpp`

``` cpp
namespace cool
{
    template <typename T>
    class colony;
}
```

### Class `cool::colony` \[Colony\]

``` cpp
template <typename T>
class colony
{
public:
    struct sentinel;

    class iterator;

    class const_iterator;

    //=== Inserts a new value into the container. ===//
    cool::colony::iterator push(T const& value);
    cool::colony::iterator push(T&& value);
    template <typename ... Args>
    cool::colony::iterator emplace(Args &&... args);

    //=== Erases an element in the container. ===//
    cool::colony::iterator erase(cool::colony::iterator it) noexcept;

    //=== Container size utilities. ===//
    std::size_t size() const;
    bool empty() const;
};
```

Colonies are unordered lists suitable for high-modification scenarios.

*Notes:* All elements within a colony have a stable memory location, that is, pointers and iterators to non-erased, non-past-end elements are valid regardless of insertions and erasures to the container and even when the container is moved.

### Struct `cool::colony::sentinel` \[Colony.\]

``` cpp
struct sentinel
{
};
```

Colony’s sentinel

-----

### Class `cool::colony::iterator` \[Colony.\]

``` cpp
class iterator
{
};
```

Stable forward iterator.

-----

### Class `cool::colony::const_iterator` \[Colony.\]

``` cpp
class const_iterator
{
};
```

Stable forward iterator of immutable elements.

-----

### Inserts a new value into the container.

``` cpp
(1) cool::colony::iterator push(T const& value);

(2) cool::colony::iterator push(T&& value);

(3) template <typename ... Args>
cool::colony::iterator emplace(Args &&... args);
```

Inserts (or constructs) a new value into the container.

*Return values:* Stable iterator to the inserted value.

*Notes:* Strong exception guarantee: both `value` and `this` is not changed if an exception occurs.

*Notes:* Always O(1) time complexity.

*Notes:* May invalidate end().

-----

### Erases an element in the container.

``` cpp
(1) cool::colony::iterator erase(cool::colony::iterator it) noexcept;
```

Erases the element pointed by `it` in the container.

*Return values:* Stable iterator to next element.

*Notes:* Never invalidates other iterators.

*Notes:* Always O(1) time complexity.

-----

### Container size utilities.

``` cpp
(1) std::size_t size() const;

(2) bool empty() const;
```

(1) Returns the container size.

(2) Returns true if the container is empty.

-----

-----
