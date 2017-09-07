#include <cool/channel.hxx>

#include <cassert>
#include <future>

using namespace cool;

auto sum(ichannel<int> ch)
{
  int x = 0, s = 0;
  while (ch >> x)
    s += x;
  return s;
}

auto write(ochannel<int> ch) { (ch << 1 << 2 << 3 << 4 << 5).close(); }

int main()
{
  auto ints = iochannel<int>();
  ints.set_limit(3u);

  auto total = std::async(std::launch::async, sum, ints);
  std::async(std::launch::async, write, ints);

  assert(total.get() == 15);

  return 0;
}
