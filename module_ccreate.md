---
title: "CCreate | Cool"
layout: default
---

# CCreate: Smart Pointer Wrapper for C APIs

Consult complete reference documentation at [cool::ccreate](doc_create.html).

## Motivation

In modern C++ development, managing memory safely is crucial. The C++ Standard
Library provides `std::unique_ptr` to automatically manage dynamically
allocated memory, preventing leaks and ensuring exception safety through RAII
(Resource Acquisition Is Initialization). However, many legacy C libraries rely
on raw pointers and require explicit deallocation using specific functions
(e.g., `std::free`, `fclose`).

Manually handling memory in such cases is error-prone, leading to potential
resource leaks or double-free errors. The `cool::ccreate` utility addresses
this problem by wrapping raw pointers in `std::unique_ptr` with a custom
deleter, ensuring that resources are automatically released when they go out of
scope.

## Solution

The `cool::ccreate` function provides a convenient and efficient way to wrap
C-style pointers in `std::unique_ptr` while specifying a custom deleter
function. It supports:

- Function pointers as deleters (e.g., `std::free`)
- Stateless lambdas, which are converted to function pointers
- Stateful lambdas for complex deallocation logic

By using `cool::ccreate`, developers can integrate legacy C APIs into modern
C++ applications with safe and efficient memory management.

## Usage Examples

### Basic Usage: Wrapping a C Library Pointer

```cpp
#include <cool/ccreate.hpp>
#include <cstdio>

int main() {
    // Create a temporary file and wrap it in a unique_ptr
    if (auto file = cool::ccreate(std::tmpfile(), std::fclose)) {
        std::fprintf(file.get(), "Hello, world!\n");
        // File is automatically closed when 'file' goes out of scope
    }
}
```

### Wrapping Heap-Allocated Memory

```cpp
#include <cool/ccreate.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>

int main() {
    const char hello[] = "Hello";
    auto buffer = cool::ccreate(static_cast<char*>(std::malloc(strlen(hello) + 1)), std::free);

    if (buffer) {
        std::strcpy(buffer.get(), hello);
        std::cout << "Buffer contains: " << buffer.get() << std::endl;
    }

    // Memory is automatically freed here
}
```

### Using a Lambda as a Custom Deleter

```cpp
#include <cool/ccreate.hpp>
#include <cstdlib>
#include <cstring>

int main() {
    const char hello[] = "Hello";
    auto buffer = cool::ccreate(static_cast<char*>(std::malloc(strlen(hello) + 1)), [](char* p) {
        std::free(p);
    });

    if (buffer) {
        std::strcpy(buffer.get(), hello);
    }

    // Custom deleter is called here

    return 0;
}
```

### Using Stateful Lambdas for Complex Cleanup

```cpp
#include <cool/ccreate.hpp>
#include <cstdlib>
#include <iostream>

int main() {
    auto rawPtr = std::malloc(10);
    if (!rawPtr) return 1;

    auto safePtr = cool::ccreate(rawPtr, [&rawPtr](void* p) {
        std::free(p);
        rawPtr = nullptr;
    });

    std::cout << "Memory safely managed." << std::endl;

    return 0; // Memory is freed and rawPtr is set to nullptr
}
```

## Summary

- `cool::ccreate` allows seamless integration of legacy C-style APIs with
  modern C++.
- It ensures proper memory management by wrapping raw pointers in
  `std::unique_ptr` with custom deleters.
- It supports function pointers, stateless lambdas (optimized to function
  pointers), and stateful lambdas.
- Using `cool::ccreate` prevents memory leaks, double-frees, and makes the code
  cleaner and safer.

By leveraging `cool::ccreate`, developers can write robust and modern C++ code
while interacting with legacy C libraries effortlessly.
