#include <cool/progress.hpp>

#include <catch.hpp>

TEST_CASE("Basic progress functionality", "[progress]")
{
  cool::progress<> pb(0.01, std::chrono::seconds{0});

  const auto ok = pb.update(0.1, [](double diff, cool::progress<>::duration elapsed) {
    CHECK(diff == Approx(0.1));
    CHECK(elapsed > cool::progress<>::duration(0));
  });

  CHECK(ok);

  CHECK_FALSE(pb.update(0.001, [](double, cool::progress<>::duration) {}));
}
