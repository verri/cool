#ifndef COOL_CCREATE_HXX_INCLUDED
/// \exclude
#define COOL_CCREATE_HXX_INCLUDED

#include <memory>
#include <type_traits>

#if __cplusplus >= 201703L
/// \exclude
#define RESULT_OF_T(F, ...) std::invoke_result_t<F, __VA_ARGS__>
#else
/// \exclude
#define RESULT_OF_T(F, ...) typename std::result_of<F(__VA_ARGS__)>::type
#endif

namespace cool
{

/// Utilities to wrap legacy C data type pointers in [std::unique_ptr]() with a
/// custom deleter.
///
/// \module CCreate
/// \group ccreate Create a wrapped C data pointer.
template <typename T, typename R> constexpr auto ccreate(T* ptr, R (*deleter)(T*)) noexcept -> std::unique_ptr<T, R (*)(T*)>
{
  return {ptr, deleter};
}

/// \module CCreate
/// \group ccreate
template <typename T, typename F, typename R = RESULT_OF_T(F, T*)>
constexpr auto ccreate(T* ptr, F deleter) noexcept ->
  typename std::enable_if<std::is_convertible<F, R (*)(T*)>::value, std::unique_ptr<T, R (*)(T*)>>::type
{
  return {ptr, deleter};
}

/// \module CCreate
/// \group ccreate
template <typename T, typename F, typename R = RESULT_OF_T(F, T*)>
constexpr auto ccreate(T* ptr, F deleter) noexcept(std::is_nothrow_move_constructible<F>::value) ->
  typename std::enable_if<!std::is_convertible<F, R (*)(T*)>::value, std::unique_ptr<T, F>>::type
{
  return {ptr, std::move(deleter)};
}

} // namespace cool

#undef RESULT_OF_T

#endif // COOL_CCREATE_HXX_INCLUDED
