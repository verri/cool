#include <cool/ccreate.hpp>

#include <catch.hpp>

#include <cstdio>

TEST_CASE("Basic ccreate functionality", "[ccreate]")
{
  auto fp1 = cool::ccreate(std::tmpfile(), std::fclose);
  auto fp2 = cool::ccreate(std::tmpfile(), +[](std::FILE* fp) {
    std::fflush(fp);
    std::fclose(fp);
  });
}
