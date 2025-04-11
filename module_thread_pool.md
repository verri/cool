---
title: "Thread Pool"
layout: default
---

# cool::thread_pool - A Simple and Intuitive Thread Pool Implementation

Consult complete reference documentation at [cool::thread_pool](doc_thread_pool.html).

## Motivation

Working with multiple threads in C++ is powerful, but also **complex and error-prone**:

- Frequently creating and destroying `std::thread` objects is inefficient.
- Manual synchronization using `mutex`, `condition_variable`, and lifecycle management requires care.
- Developers often just want to **run multiple tasks in parallel**, without dealing with low-level thread management.

### The Problem: How can we manage concurrent tasks safely and efficiently?

---

## Solution: `cool::thread_pool`

The `cool::thread_pool` class provides a **lightweight and modern abstraction** for concurrent task execution:

- Maintains a **fixed pool of worker threads** running in the background.
- Allows dynamic **task enqueuing**.
- Tasks are executed in parallel as soon as threads become available.
- Uses **`std::future`** so the caller can retrieve task results later.
- Provides explicit control: `close`, `join`, and `detach`.

---

## How it Works

### Internal Structure:

- A **vector of threads** (`workers_`) continuously runs tasks pulled from a **mutex-protected queue** (`tasks_`).
- Each task is wrapped as a `std::function<void()>`.
- Threads block on a `std::condition_variable` until a new task is available.
- When `close()` is called, no new tasks can be added.
- `join()` waits for all worker threads to finish safely.

---

## Usage Example

### 1. Creating a `thread_pool` and executing tasks

```cpp
#include <cool/thread_pool.hpp>
#include <iostream>

int main() {
  const std::size_t nthreads = 4; // Number of threads in the pool
  cool::thread_pool pool(nthreads);

  // Enqueue asynchronous tasks
  auto future1 = pool.enqueue([] { return 42; });
  auto future2 = pool.enqueue([](int a, int b) { return a + b; }, 10, 5);

  std::cout << "Result 1: " << future1.get() << "\n"; // 42
  std::cout << "Result 2: " << future2.get() << "\n"; // 15

  pool.join(); // Waits for all threads to finish
}
```

### 2. Explicit control with `close`, `join`, `detach`

```cpp
cool::thread_pool pool;

auto f = pool.enqueue([] { return 1 + 2; });
std::cout << f.get() << "\n"; // 3

pool.close(); // No new tasks allowed
// pool.enqueue([] { return 9; }); // Throws exception!

pool.join(); // Waits for threads to terminate
```

---

## Additional Features

- If `nthreads` is not specified, it defaults to `std::thread::hardware_concurrency()`.
- Uses `std::packaged_task` to wrap arbitrary function calls.
- Compatible with both C++11 and C++17 via `std::result_of` / `std::invoke_result`.

---

## When to Use

Use `cool::thread_pool` when you:

- Need to run multiple tasks concurrently in a safe and reusable way.
- Want to avoid the complexity of manually managing `std::thread`.
- Prefer integrating with `std::future` for retrieving results.

---

## When **Not** to Use

- If you need **short-lived, fire-and-forget threads**, consider `std::async` or `std::jthread`.
- If your application requires **task prioritization** or **custom scheduling**, this thread pool may be too simple.

---

## Conclusion

`cool::thread_pool` offers an elegant and practical way to leverage **parallel execution in C++**, with **minimal boilerplate** and **maximum safety**.
