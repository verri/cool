#ifndef COOL_DEFER_HXX_INCLUDED
/// \exclude
#define COOL_DEFER_HXX_INCLUDED

#include <utility>

namespace cool
{

/// \exclude
namespace detail
{
template <typename F> struct defer_helper {
  defer_helper(F f) : f_{std::move(f)} {}

  defer_helper(const defer_helper&) = delete;
  defer_helper(defer_helper&&) noexcept = default;

  auto operator=(const defer_helper&) -> defer_helper& = delete;
  auto operator=(defer_helper&&) noexcept -> defer_helper& = default;

  ~defer_helper() { f_(); }

  F f_;
};

} // namespace detail

template <typename F> auto defer(F&& f) -> detail::defer_helper<F> { return {std::forward<F>(f)}; }

#define COOL_TOKEN_CONCAT(X, Y) X##Y
#define COOL_TOKEN_PASTE(X, Y) COOL_TOKEN_CONCAT(X, Y)

#define COOL_DEFER(...) auto COOL_TOKEN_PASTE(_deferred, __LINE__) = ::cool::defer([&] { __VA_ARGS__; })

} // namespace cool

#endif // COOL_DEFER_HXX_INCLUDED
