#include <cool/thread_pool.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic thread_pool functionalities", "[thread_pool]")
{
  using namespace cool;
  const auto sum = [](int x, int y) { return x + y; };

  {
    thread_pool pool;
    pool.join(); // Pool is joinable like a thread.
  }

  {
    thread_pool pool;
    CHECK_FALSE(pool.is_closed());

    // Tasks are packed into a future.
    auto result = pool.enqueue(sum, 10, 7);
    CHECK(result.get() == 17); // `get` blocks the current thread but not the pool.

    // Join is required, unless you detach the thread pool.
    pool.join(); // `join` closes the pool.

    CHECK(pool.is_closed());
  }

  {
    thread_pool pool;
    CHECK_FALSE(pool.is_closed());

    auto result = pool.enqueue(sum, 1, 2);
    CHECK_FALSE(pool.is_closed());

    pool.close(); // `close` doesn't join the pool.
    CHECK(pool.is_closed());

    CHECK(result.get() == 3);
    CHECK_THROWS(pool.enqueue(sum, 3, 5));

    pool.join(); // `join` closes the pool.
  }
}

TEST_CASE("Thread pool constructor tests", "[thread_pool]")
{
  using namespace cool;

  // Default constructor (uses hardware_concurrency)
  {
    thread_pool pool;
    CHECK_FALSE(pool.is_closed());
    pool.join();
  }

  // Constructor with explicit thread count
  {
    thread_pool pool(2);
    CHECK_FALSE(pool.is_closed());
    pool.join();
  }

  // Constructor with zero threads (should use hardware_concurrency)
  {
    thread_pool pool(0);
    CHECK_FALSE(pool.is_closed());
    pool.join();
  }
}

TEST_CASE("Thread pool exception safety", "[thread_pool]")
{
  using namespace cool;

  // Test basic pool operation
  {
    thread_pool pool(1);

    auto result = pool.enqueue([]() { return 42; });
    CHECK(result.get() == 42);

    pool.join();
  }

  // Test multiple tasks
  {
    thread_pool pool(2);

    std::vector<std::future<int>> futures;

    for (int i = 0; i < 5; ++i) {
      futures.push_back(pool.enqueue([i]() { return i * 2; }));
    }

    // Check results
    for (int i = 0; i < 5; ++i) {
      CHECK(futures[i].get() == i * 2);
    }

    pool.join();
  }
}
