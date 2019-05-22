#include <cool/task_manager.hpp>

#include <catch.hpp>

TEST_CASE("Basic task_manager functionalities", "[task_manager]")
{
  using namespace cool;
  const auto sum = [](int x, int y) { return x + y; };

  {
    task_manager pool;
    pool.join(); // Pool is joinable like a thread.
  }

  {
    task_manager pool;
    CHECK_FALSE(pool.is_closed());

    // Tasks are packed into a future.
    auto result = pool.enqueue(0, sum, 10, 7);
    CHECK(result.get() == 17); // `get` blocks the current thread but not the pool.

    // Join is required, unless you detach the thread pool.
    pool.join(); // `join` closes the pool.

    CHECK(pool.is_closed());
  }

  {
    task_manager pool;
    CHECK_FALSE(pool.is_closed());

    auto result = pool.enqueue(0, sum, 1, 2);
    CHECK_FALSE(pool.is_closed());

    pool.close(); // `close` doesn't join the pool.
    CHECK(pool.is_closed());

    CHECK(result.get() == 3);
    CHECK_THROWS(pool.enqueue(0, sum, 3, 5));

    pool.join(); // `join` closes the pool.
  }
}
