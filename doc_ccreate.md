---
---

# Header file `ccreate.hpp`

``` cpp
namespace cool
{
    template <typename T, typename R>
    constexpr std::unique_ptr<T, R (*)(T *)> ccreate(T*, T* ptr, R(* deleter)(T*)) noexcept;

    template <typename T, typename F, typename R = typename std::result_of<F(T*)>::type>
    constexpr typename std::enable_if<std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, R (*)(T *)> >::type ccreate(T*, T* ptr, F deleter) noexcept;

    template <typename T, typename F, typename R = typename std::result_of<F(T*)>::type>
    constexpr typename std::enable_if<!std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, F> >::type ccreate(T* ptr, F deleter) noexcept(std::is_nothrow_move_constructible<F>::value);
}
```
