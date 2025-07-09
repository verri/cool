# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Cool is a header-only C++ library providing common standalone utilities missing from the standard library. It consists of self-contained headers, each implementing a specific utility. The library supports C++11 and above, with some features requiring C++17 or later.

## Core Components

The library contains the following main utilities:

- **colony**: Unordered container with stable memory addresses for high-modification scenarios
- **channel**: Go-like channels for thread communication
- **thread_pool**: Thread pool implementation with job queuing
- **defer**: Deferred execution of statements (similar to Go's defer)
- **compose**: Lambda composition utility (C++17+ only)
- **enum_map**: Enumeration-based mapping (C++17+ only)
- **indices**: Utility for safer range-based for loops
- **ccreate**: Wrapper for legacy C data types
- **progress**: Progress tracking utility

## Build System

The project uses CMake as its primary build system:

### Basic Build Commands

```bash
# Configure with default C++ standard (C++11)
cmake -S. -Bbuild

# Build the project
cmake --build build

# Install system-wide
cmake --install build
```

### Building with Different C++ Standards

The project supports multiple C++ standards. Use separate build directories:

```bash
# C++11 (default)
cmake -S. -Bbuild-c++11 -DCOOL_TEST_STANDARD=11

# C++14
cmake -S. -Bbuild-c++14 -DCOOL_TEST_STANDARD=14

# C++17 (enables compose and enum_map)
cmake -S. -Bbuild-c++17 -DCOOL_TEST_STANDARD=17

# C++20
cmake -S. -Bbuild-c++20 -DCOOL_TEST_STANDARD=20

# C++23
cmake -S. -Bbuild-c++23 -DCOOL_TEST_STANDARD=23
```

### Testing

```bash
# Enable testing during configuration
cmake -S. -Bbuild -DCOOL_BUILD_TEST=ON

# Build and run tests
cmake --build build

# Run test executable directly
./build/test/cool_test_suite
```

### Test Coverage

```bash
# Configure with coverage
cmake -S. -Bbuild -DCOOL_BUILD_TEST=ON -DCOOL_TEST_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

# Build and run tests
cmake --build build
```

## Development Commands

### Code Formatting and Linting

```bash
# Format code using clang-format
make format

# Run clang-tidy on headers
make tidy

# Clean coverage files
make clean
```

## Architecture Notes

- **Header-only**: All functionality is implemented in headers under `include/cool/`
- **Thread Safety**: Components like `channel` and `thread_pool` use mutexes and condition variables for thread-safe operations
- **C++ Standard Compatibility**: The library uses feature detection macros to provide compatibility across different C++ standards
- **Testing**: Uses Catch2 v3.8.0 for unit testing with comprehensive test coverage
- **Memory Management**: Components maintain stable memory addresses (especially `colony`) and use RAII patterns
- **Template-heavy**: Most utilities are template-based for type safety and performance

## Usage Patterns

When using this library:
- Include individual headers as needed (e.g., `#include <cool/colony.hpp>`)
- Link against threads library when using `channel` or `thread_pool`
- Use CMake target `cool` for easy integration: `target_link_libraries(target cool)`
- Check C++ standard requirements for specific features (compose/enum_map require C++17+)