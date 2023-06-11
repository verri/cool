#include <cool/channel.hpp>

#include <future>

#include <catch2/catch_test_macros.hpp>

using namespace cool;

TEST_CASE("Basic channel functionality", "[channel]")
{
  // cool::channel is a pipe that can receive and send data among different threads in a
  // safe way.

  // A channel can send/receive data of a specific type.
  {
    // An int channel.
    auto ich = channel<int>();
    // A float channel.
    auto fch = channel<float>();
  }

  // A buffer size limit can be given at runtime.
  // If the buffer is overfilled, the current thread is blocked.
  {
    auto ch = channel<int>(10u);
    CHECK(ch.buffer_size() == 10u);

    ch.buffer_size(20u);
    CHECK(ch.buffer_size() == 20u);
  }

  // A copy of a channel refers to the same channel.
  // A channel object can be assigned.
  {
    auto ch1 = channel<int>();
    auto ch2 = ch1;
    auto ch3 = channel<int>();

    CHECK(ch1 == ch2);
    CHECK(ch1 != ch3);

    CHECK_FALSE(ch1 != ch2);
    CHECK_FALSE(ch1 == ch3);

    ch3 = ch1;
    CHECK(ch1 == ch3);
    CHECK_FALSE(ch1 != ch3);

    auto ch4 = std::move(ch1);
    // ch1 is in an unspecified state.
    ch1 = ch2;
    // ch1 is "reinitialized"

    CHECK(ch4 == ch2);
    CHECK(ch1 == ch2);
    CHECK_FALSE(ch4 != ch2);
    CHECK_FALSE(ch1 != ch2);
  }

  // Data transmission.
  {
    auto ch = channel<int>();
    ch.send(10);
    CHECK(ch.receive() == 10);
  }

  // Data piping.
  {
    auto ch = channel<int>();
    int x = 0, y = 10;
    ch << y;
    ch >> x;
    CHECK(x == y);
  }

  // Input and output operations can be restricted.
  {
    channel<int> ch; // sends and receives

    ichannel<int> ich = ch; // only receives
    CHECK(ich == ch);
    CHECK(ch == ich);
    CHECK_FALSE(ich != ch);
    CHECK_FALSE(ch != ich);

    ochannel<int> och = ch; // only sends
    CHECK(och == ch);
    CHECK(ch == och);
    CHECK_FALSE(och != ch);
    CHECK_FALSE(ch != och);

    CHECK(och == ich);
    CHECK(ich == och);
    CHECK_FALSE(och != ich);
    CHECK_FALSE(ich != och);

    och.send(10);
    CHECK(ich.receive() == 10);
  }

  // Example of safe usage.
  {
    auto ch = channel<int>(3u);

    const auto sum = [](ichannel<int> ch) {
      int x = 0, s = 0;
      while (ch >> x) // when a piping operation fail, returns false.
        s += x;
      return s;
    };

    const auto write = [](ochannel<int> ch) {
      // When a cool::ochannel receives cool::eod, the channel is closed.
      ch << 1 << 2 << 3 << 4 << 5 << eod;
    };

    auto total = std::async(std::launch::async, sum, ch);
    auto thr = std::thread([&total]() { CHECK(total.get() == 15); });
    {
      auto _ = std::async(std::launch::async, write, ch);
      (void)_;
    }

    thr.join();

    // After closed, a channel cannot receive more data.
    int x = 1;

    CHECK_THROWS(ch.send(1));
    CHECK_THROWS(ch.send(x));

    CHECK_FALSE(ch << 1);
    CHECK_FALSE(ch << x);
  }

  {
    // A default-constructed channel has virtually infinity buffer and is opened.
    auto ch = channel<int>();
    CHECK_FALSE(ch.is_closed());
    CHECK(ch.buffer_size() == std::numeric_limits<std::size_t>::max());

    ch << 10;

    // Buffer size can be changed after receiving data.
    ch.buffer_size(2u);
    CHECK(ch.buffer_size() == 2u);

    // There is no problem in closing a channel with data.
    ch.close();
    CHECK(ch.is_closed());

    // But we cannot put more data in it.
    CHECK_FALSE(ch << 1);
    CHECK_FALSE(ch);

    // But we can retrieve it.
    int x = 1;
    CHECK(ch >> x);
    CHECK(x == 10);

    // Now it is empty.
    CHECK_FALSE(ch >> x);
    CHECK(x == 10);
    CHECK_FALSE(ch);
  }

  {
    auto ch = channel<int>();
    CHECK(ch.wait_for(std::chrono::milliseconds{1}, [](int) { CHECK(false); }) == std::cv_status::timeout);
    ch << 1;
    CHECK(ch.wait_for(std::chrono::milliseconds{1}, [](int i) { CHECK(i == 1); }) == std::cv_status::no_timeout);
  }
}
