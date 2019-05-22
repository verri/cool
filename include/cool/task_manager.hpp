#ifndef COOL_TASK_MANAGER_HPP_INCLUDED
#define COOL_TASK_MANAGER_HPP_INCLUDED

#include <cool/indices.hpp>

#include <algorithm>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

#if __cplusplus >= 201703L
/// \exclude
#define RESULT_OF_T(F, ...) std::invoke_result_t<F, __VA_ARGS__>
#else
/// \exclude
#define RESULT_OF_T(F, ...) typename std::result_of<F(__VA_ARGS__)>::type
#endif

#if __cpp_constexpr > 201703L // heap algorithms are constexpr only in C++20
#define RELAXED_CONSTEXPR constexpr
#else
#define RELAXED_CONSTEXPR
#endif

namespace cool
{

namespace detail
{
template <typename RandomIt>
RELAXED_CONSTEXPR void update_heap(RandomIt first, RandomIt last, RandomIt updated)
{
  // Sift up
  auto current = updated;
  while (current != first)
  {
    auto parent = first + (std::distance(first, current) - 1) / 2;
    if (*current <= *parent)
      break;

    std::iter_swap(current, parent);
    current = parent;
  }

  // Sift down
  while (true)
  {
    auto maximum = current;
    const auto lchild = first + 2 * std::distance(first, current) + 1;
    const auto rchild = first + 2 * std::distance(first, current) + 2;

    if (lchild < last && *lchild > *maximum)
      maximum = lchild;

    if (rchild < last && *rchild > *maximum)
      maximum = rchild;

    if (maximum == current)
      break;

    std::iter_swap(current, maximum);
    current = maximum;
  }
}
} // namespace detail

class closed_task_manager : public std::system_error
{
  using std::system_error::system_error;
};

class task_manager
{
private:
  struct task {
    int priority;
    std::function<void()> job;

    void operator()() const { job(); }
    bool operator<(const task& other) const { return priority < other.priority; }
  };

public:
  explicit task_manager(std::size_t nthreads = 0)
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

            std::pop_heap(tasks_.begin(), tasks_.end());
            task = std::move(tasks_.back());
            tasks_.pop_back();
          }
          task();
        }
      });
    }
  }

  task_manager(const task_manager&) = delete;
  task_manager(task_manager&&) = delete;

  auto operator=(const task_manager&) -> task_manager& = delete;
  auto operator=(task_manager &&) -> task_manager& = delete;

  template <typename F, typename... Args> auto enqueue(int priority, F&& f, Args&&... args) -> std::future<RESULT_OF_T(F&&, Args&&...)>
  {
    using ptask_t = std::packaged_task<RESULT_OF_T(F&&, Args && ...)()>;

    auto task = std::make_shared<ptask_t>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto result = task->get_future();

    {
      auto lock = std::unique_lock<std::mutex>(mutex_);
      if (closed_)
        throw closed_task_manager{std::make_error_code(std::errc::invalid_argument), "enqueue on closed task_manager"};

      tasks_.push_back({priority, [task] { (*task)(); }});
      std::push_heap(tasks_.begin(), tasks_.end());
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

  std::vector<task> tasks_;
  std::vector<std::thread> workers_;

  std::condition_variable cv_;
  mutable std::mutex mutex_;

  bool closed_ = false;
};

} // namespace cool

#undef RESULT_OF_T

#endif // COOL_TASK_MANAGER_HPP_INCLUDED
