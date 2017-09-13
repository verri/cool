---
---

# Header file `indices.hpp`<a id="indices.hpp"></a>

``` cpp
namespace cool
{
    template <typename T>
    class index_iterator;
    
    template <typename T>
    class index_range;
    
    template <typename T>
    index_range<T> indices(T value);
    
    template <typename T>
    index_range<T> indices(T begin, T end);
}
```
