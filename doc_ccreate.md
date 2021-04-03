---
---

# Header file `ccreate.hpp`

``` cpp
namespace cool
{
    //=== Create a wrapped C data pointer. ===//
    template <typename T, typename R>
    constexpr std::unique_ptr<T, R (*)(T *)> ccreate(T* ptr, R(* deleter)(T*), T*) noexcept;
    template <typename T, typename F, typename R = std::invoke_result_t<F,T*>>
    constexpr typename std::enable_if<std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, R (*)(T *)>>::type ccreate(T* ptr, F deleter, T*) noexcept;
    template <typename T, typename F, typename R = std::invoke_result_t<F,T*>>
    constexpr typename std::enable_if<!std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, F>>::type ccreate(T* ptr, F deleter) noexcept(std::is_nothrow_move_constructible<F>::value);
}
```

### Create a wrapped C data pointer. \[CCreate\]

``` cpp
(1) template <typename T, typename R>
constexpr std::unique_ptr<T, R (*)(T *)> ccreate(T* ptr, R(* deleter)(T*), T*) noexcept;

(2) template <typename T, typename F, typename R = std::invoke_result_t<F,T*>>
constexpr typename std::enable_if<std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, R (*)(T *)>>::type ccreate(T* ptr, F deleter, T*) noexcept;

(3) template <typename T, typename F, typename R = std::invoke_result_t<F,T*>>
constexpr typename std::enable_if<!std::is_convertible<F, R (*)(T *)>::value, std::unique_ptr<T, F>>::type ccreate(T* ptr, F deleter) noexcept(std::is_nothrow_move_constructible<F>::value);
```

Utilities to wrap legacy C data type pointers in [`std::unique_ptr`](http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=std::unique_ptr) with a custom deleter.

-----
