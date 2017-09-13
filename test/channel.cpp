#include <cool/channel.hpp>

#include <future>

#include <catch.hpp>

using namespace cool;

int sum(ichannel<int> ch)
{
  int x = 0, s = 0;
  while (ch >> x)
    s += x;
  return s;
}

void write(ochannel<int> ch) { ch << 1 << 2 << 3 << 4 << 5 << eod; }

TEST_CASE("Basic channel functionality", "[channel]")
{
  auto ints = channel<int>(3u);

  auto total = std::async(std::launch::async, sum, ints);
  auto thr = std::thread([&total]() { CHECK(total.get() == 15); });
  std::async(std::launch::async, write, ints);

  thr.join();
}
