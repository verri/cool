#ifndef COOL_THREAD_POOL_HPP_INCLUDED
#define COOL_THREAD_POOL_HPP_INCLUDED

#include <cool/indices.hpp>

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>

#if __cplusplus >= 201703L
/// \exclude
#define RESULT_OF_T(F, ...) std::invoke_result_t<F, __VA_ARGS__>
#else
/// \exclude
#define RESULT_OF_T(F, ...) typename std::result_of<F(__VA_ARGS__)>::type
#endif

namespace cool
{

class closed_thread_pool : public std::system_error
{
  using std::system_error::system_error;
};

class thread_pool
{
public:
  explicit thread_pool(std::size_t nthreads = 0)
  {
    if (nthreads == 0)
      nthreads = std::thread::hardware_concurrency();

    for (auto _ : indices(nthreads)) {
      (void)_;

      workers_.emplace_back([this] {
        while (true) {
          auto task = std::function<void()>();
          {
            auto lock = std::unique_lock<std::mutex>(mutex_);
            cv_.wait(lock, [this] { return closed_ || !tasks_.empty(); });

            if (closed_ && tasks_.empty())
              return;

            task = std::move(tasks_.front());
            tasks_.pop();
          }
          task();
        }
      });
    }
  }

  thread_pool(const thread_pool&) = delete;
  thread_pool(thread_pool&&) = delete;

  auto operator=(const thread_pool&) -> thread_pool& = delete;
  auto operator=(thread_pool &&) -> thread_pool& = delete;

  template <typename F, typename... Args> auto enqueue(F&& f, Args&&... args) -> std::future<RESULT_OF_T(F&&, Args&&...)>
  {
    using ptask_t = std::packaged_task<RESULT_OF_T(F&&, Args && ...)()>;

    auto task = std::make_shared<ptask_t>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto result = task->get_future();

    {
      auto lock = std::unique_lock<std::mutex>(mutex_);
      if (closed_)
        throw closed_thread_pool{std::make_error_code(std::errc::invalid_argument), "enqueue on closed thread_pool"};

      tasks_.emplace([task] { (*task)(); });
    }
    cv_.notify_one();

    return result;
  }

  auto join() -> void
  {
    close();
    for (auto& worker : workers_)
      worker.join();
  }

  auto detach() -> void
  {
    for (auto& worker : workers_)
      worker.detach();
  }

  auto joinable() const noexcept -> bool
  {
    auto l = lock();
    return workers_.back().joinable();
  }

  auto close() noexcept -> void
  {
    auto l = lock();
    closed_ = true;
    cv_.notify_all();
  }

  auto is_closed() const noexcept -> bool
  {
    auto l = lock();
    return closed_;
  }

private:
  auto lock() const -> std::unique_lock<std::mutex> { return std::unique_lock<std::mutex>(mutex_); }

  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;

  std::condition_variable cv_;
  mutable std::mutex mutex_;

  bool closed_ = false;
};

} // namespace cool

#undef RESULT_OF_T

#endif // COOL_THREAD_POOL_HPP_INCLUDED
