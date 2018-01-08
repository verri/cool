#include <cool/ccreate.hpp>

#include <catch.hpp>

#include <cstdio>
#include <cstdlib>

TEST_CASE("Basic ccreate functionality", "[ccreate]")
{
  auto fp1 = cool::ccreate(std::tmpfile(), std::fclose);
  auto fp2 = cool::ccreate(std::tmpfile(), [](std::FILE* fp) {
    std::fflush(fp);
    std::fclose(fp);
  });

  auto p = cool::ccreate(std::malloc(1), std::free);

  int i = 0;
  {
    cool::ccreate(std::malloc(1), [&i](void* p) {
      std::free(p);
      i = 1;
    });
  }
  CHECK(i == 1);
}
