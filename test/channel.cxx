#include <cool/channel.hxx>

#include <iostream>
#include <thread>

using namespace cool;

int main() {
  iochannel<int> ints;
  ints.set_limit(3u);

  std::cout << "is_closed() = " << std::boolalpha << ints.is_closed() << '\n';
  std::cout << "limit() = " << ints.limit() << '\n';

  std::thread ithr([i = 0](ichannel<int> ch) mutable -> void {
    int j;
    while (ch >> j)
    {
      std::cout << "read " << j << '\n';
      if (++i != j)
        std::cerr << i << " != " << j << '\n';
    }
  }, ints);

  std::thread othr([](ochannel<int> ch) {
    ch << 1 << 2 << 3 << 4 << 5;
    ch.close();
  }, ints);

  othr.join();
  ithr.join();

  return 0;
}
