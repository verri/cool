#ifndef COOL_COMPOSE_HPP_INCLUDED
#define COOL_COMPOSE_HPP_INCLUDED

#include <type_traits>
#include <utility>

static_assert(__cplusplus >= 201703L, "cool::compose requires C++17");

namespace cool
{

template <typename... F>
struct compose : F...
{
  explicit constexpr compose(F... f) noexcept((std::is_nothrow_move_constructible_v<F> && ...)) : F(std::move(f))... {}
  using F::operator()...;
};

} // namespace cool

#endif // COOL_COMPOSE_HPP_INCLUDED
