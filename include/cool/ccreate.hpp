#ifndef COOL_CCREATE_HXX_INCLUDED
/// \exclude
#define COOL_CCREATE_HXX_INCLUDED

#include <memory>
#include <type_traits>

namespace cool
{

namespace detail
{
#if __cplusplus >= 201703L
template <typename F, typename... Args> using result_of = std::invoke_result<F, Args...>;
#else
template <typename F, typename... Args> using result_of = std::result_of<F(Args...)>;
#endif
}

template <typename T, typename R> constexpr auto ccreate(T* ptr, R (*deleter)(T*)) noexcept -> std::unique_ptr<T, R (*)(T*)>
{
  return {ptr, deleter};
}

template <typename T, typename F, typename R = typename detail::result_of<F, T*>::type>
constexpr auto ccreate(T* ptr, F deleter) noexcept ->
  typename std::enable_if<std::is_convertible<F, R (*)(T*)>::value, std::unique_ptr<T, R (*)(T*)>>::type
{
  return {ptr, deleter};
}

template <typename T, typename F, typename R = typename detail::result_of<F, T*>::type>
constexpr auto ccreate(T* ptr, F deleter) noexcept(std::is_nothrow_move_constructible<F>::value) ->
  typename std::enable_if<!std::is_convertible<F, R (*)(T*)>::value, std::unique_ptr<T, F>>::type
{
  return {ptr, std::move(deleter)};
}

} // namespace cool

#endif // COOL_CCREATE_HXX_INCLUDED
