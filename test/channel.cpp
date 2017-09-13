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
  auto ch = channel<int>(3u);

  auto total = std::async(std::launch::async, sum, ch);
  auto thr = std::thread([&total]() { CHECK(total.get() == 15); });
  std::async(std::launch::async, write, ch);

  thr.join();

  CHECK_THROWS(ch.send(1));

  auto ch2 = channel<int>();
  CHECK(ch);
  CHECK_FALSE(ch2.is_closed());
  CHECK(ch2.buffer_size() == std::numeric_limits<std::size_t>::max());

  ch2.buffer_size(2u);
  CHECK(ch2.buffer_size() == 2u);

  int x = 1;
  ch2 << x;

  ch2.close();
  CHECK(ch2.is_closed());

  CHECK_FALSE(ch2 << x);
  CHECK_FALSE(ch2 << 1);
  CHECK_FALSE(ch2);

  CHECK(ch2 >> x);
  CHECK_FALSE(ch2 >> x);
  CHECK_FALSE(ch2);
}
