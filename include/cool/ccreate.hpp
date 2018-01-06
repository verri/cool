#ifndef COOL_CCREATE_HXX_INCLUDED
/// \exclude
#define COOL_CCREATE_HXX_INCLUDED

#include <memory>

namespace cool
{

template <typename T, typename R> auto ccreate(T* ptr, R (*deleter)(T*)) -> std::unique_ptr<T, R (*)(T*)>
{
  return {ptr, deleter};
}

} // namespace cool

#endif // COOL_CCREATE_HXX_INCLUDED
