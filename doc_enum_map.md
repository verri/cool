---
---

# Header file `enum_map.hpp`

``` cpp
namespace cool
{
    template <auto V>constexpr detail::enum_key_t<V>enum_key;

    template <typename E, typename T>
    class enum_map_iterator;

    template <typename T, auto V, auto Vs ... >
    class enum_map;
}
```
