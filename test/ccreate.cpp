#include <cool/ccreate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

TEST_CASE("Basic ccreate functionality", "[ccreate]")
{
  // cool::ccreate is intended to be used with legacy C data types the are constructed by a
  // function that returns a pointer and destroyed by a second function.
  // The pointer is wrapped in a std::unique_ptr, guaranteeing correct RAII behavior.

  // The basic usage is
  {
    auto fp = cool::ccreate(std::tmpfile(), std::fclose);
    std::fprintf(fp.get(), "Hello"); // FILE* can be retrieved by std::unique_ptr::get.
  } // fp is closed here.
  {
    const char hello[] = "Hello";
    auto p = cool::ccreate(std::malloc(sizeof(char) * (1 + std::strlen(hello))), std::free);

    std::strcpy((char*)p.get(), hello);
    CHECK(std::strcmp((char*)p.get(), hello) == 0);
  } // p is freed here.

  // The argument of the "destructor" must match the type of the first argument of
  // ccreate.  A lambda can be used to guarantee that.
  {
    const char hello[] = "Hello";
    auto p = cool::ccreate((char*)std::malloc(sizeof(char) * (1 + std::strlen(hello))), [](char* p) { free((void*)p); });

    std::strcpy(p.get(), hello);
    CHECK(std::strcmp(p.get(), hello) == 0);

    // Stateless lambdas are automatically converted to function pointer.
    // That should reduce binary size.
    static_assert(std::is_same<decltype(p), std::unique_ptr<char, void (*)(char*)>>::value, "");
  }

  // Stateful lambdas, although not recommended, are also allowed.
  {
    auto unsafe = std::malloc(1);
    REQUIRE(unsafe != nullptr);
    {
      auto safe = cool::ccreate(unsafe, [&unsafe](decltype(unsafe)) {
        std::free(unsafe);
        unsafe = nullptr;
      });

      // Stateful destructors are not convertible to function pointers.
      static_assert(!std::is_same<decltype(safe), std::unique_ptr<void, void (*)(void*)>>::value, "");
    }
    CHECK(unsafe == nullptr);
  }
}
