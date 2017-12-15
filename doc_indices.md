---
---

# Header file `indices.hpp`

``` cpp
#include <iterator>

namespace cool
{
    template <typename T>
    class index_iterator;

    template <typename T>
    class index_range;

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, index_range<T> >::type indices(T value);

    template <typename T, typename U>
    typename std::enable_if<std::is_integral<T>::value && std::is_integral<U>::value, index_range<typename std::common_type<T, U>::type> >::type indices(T begin, U end);
}
```
