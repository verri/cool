#ifndef COOL_THREAD_POOL_HPP_INCLUDED
#define COOL_THREAD_POOL_HPP_INCLUDED

#include <cool/indices.hpp>

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace cool
{

namespace detail
{

template <typename T> struct infer_queue {
  struct task_t {
    T key;
    std::function<void()> fn;

    template <typename F> task_t(const T& key, F&& fn) : key{key}, fn{fn} {}

    auto operator<(const task_t other) const -> bool { return key < other.key; }
    auto operator()() const -> void { fn(); }
  };

  using queue_t = std::priority_queue<task_t>;
  static constexpr auto is_priority = true;
  static auto dequeue(const queue_t& queue) -> const task_t& { return queue.top(); }
};

template <> struct infer_queue<std::nullptr_t> {
  using task_t = std::function<void()>;
  using queue_t = std::queue<task_t>;
  static constexpr auto is_priority = false;
  static auto dequeue(queue_t& queue) -> task_t& { return queue.front(); }
};

} // namespace detail

template <typename T = std::nullptr_t> class priority_thread_pool
{
  using helper_t = detail::infer_queue<T>;
  using queue_t = typename helper_t::queue_t;

public:
  priority_thread_pool(std::size_t nthreads = 0);
  ~priority_thread_pool();

  priority_thread_pool(const priority_thread_pool&) = delete;
  priority_thread_pool(priority_thread_pool&&) = delete;

  auto operator=(const priority_thread_pool&) -> priority_thread_pool& = delete;
  auto operator=(priority_thread_pool &&) -> priority_thread_pool& = delete;

  template <typename F, typename... Args, typename R = typename std::result_of<F(Args...)>::type>
  auto enqueue(F&& f, Args&&... args) -> std::future<R>;

  template <typename F, typename... Args, typename R = typename std::result_of<F(Args...)>::type>
  auto enqueue(const T& priority, F&& f, Args&&... args) -> std::future<R>;

  auto join_all() noexcept -> void;

private:
  queue_t tasks;
  std::vector<std::thread> workers;

  std::condition_variable task_condition;
  std::mutex queue_mutex;

  bool stopped = false;
};

template <typename T> priority_thread_pool<T>::priority_thread_pool(std::size_t nthreads)
{
  if (nthreads == 0)
    nthreads = std::thread::hardware_concurrency();

  for (auto _ : indices(nthreads)) {
    (void)_;

    workers.emplace_back([this] {
      while (true) {
        auto task = std::function<void()>();
        {
          auto lock = std::unique_lock<std::mutex>(queue_mutex);
          task_condition.wait(lock, [this] { return stopped || !tasks.empty(); });

          if (stopped && tasks.empty())
            return;

          task = std::move(helper_t::dequeue(tasks));
          tasks.pop();
        }
        task();
      }
    });
  }
}

template <typename T> priority_thread_pool<T>::~priority_thread_pool()
{
  {
    auto lock = std::unique_lock<std::mutex>(queue_mutex);
    stopped = true;
  }

  task_condition.notify_all();
  for (auto&& worker : workers)
    if (worker.joinable())
      worker.join();
}

template <typename T>
template <typename F, typename... Args, typename R>
auto priority_thread_pool<T>::enqueue(F&& f, Args&&... args) -> std::future<R>
{
  static_assert(std::is_same<std::nullptr_t, T>::value, "a priority must be assigned for tasks in this pool");

  auto task = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  auto res = task->get_future();

  {
    auto lock = std::unique_lock<std::mutex>(queue_mutex);
    if (stopped)
      throw std::runtime_error{"enqueue on stopped priority_thread_pool"};

    tasks.emplace([task] { (*task)(); });
  }
  task_condition.notify_one();

  return res;
}

template <typename T>
template <typename F, typename... Args, typename R>
auto priority_thread_pool<T>::enqueue(const T& priority, F&& f, Args&&... args) -> std::future<R>
{
  static_assert(!std::is_same<std::nullptr_t, T>::value, "a priority cannot be assigned for tasks in this pool");

  auto task = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  auto res = task->get_future();

  {
    auto lock = std::unique_lock<std::mutex>(queue_mutex);
    if (stopped)
      throw std::runtime_error{"enqueue on stopped priority_thread_pool"};
    tasks.emplace(priority, [task] { (*task)(); });
  }
  task_condition.notify_one();

  return res;
}

template <typename T> auto priority_thread_pool<T>::join_all() noexcept -> void
{
  {
    auto lock = std::unique_lock<std::mutex>(queue_mutex);
    stopped = true;
  }

  task_condition.notify_all();
  for (auto&& worker : workers)
    worker.join();
}

using thread_pool = priority_thread_pool<>;

} // namespace cool

#endif // COOL_THREAD_POOL_HPP_INCLUDED
