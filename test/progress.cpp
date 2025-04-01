#include <cool/progress.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic progress functionality", "[progress]")
{
  {
    cool::progress<> pb(0.01, std::chrono::seconds{0});

    CHECK(pb.update(0.1, [](double diff, cool::progress<>::duration elapsed) {
      CHECK(diff == Catch::Approx(0.1));
      CHECK(elapsed > cool::progress<>::duration(0));
    }));

    CHECK_FALSE(pb.update(0.001, [](double, cool::progress<>::duration) {}));
  }

  {
    cool::progress<> pb(0, std::chrono::hours{1});

    CHECK_FALSE(pb.update(1, [](double diff, cool::progress<>::duration elapsed) {
      CHECK(diff == Catch::Approx(1.0));
      CHECK(elapsed < std::chrono::hours{1});
    }));
  }
}
