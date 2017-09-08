#include <cool/thread_pool.hpp>

#include <catch.hpp>

TEST_CASE("Basic thread_pool functionalities", "[thread_pool]")
{
  cool::thread_pool pool;
  pool.join_all();
}
