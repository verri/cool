# Cool

[![GitHub license](https://img.shields.io/badge/license-Zlib-blue.svg)](https://raw.githubusercontent.com/verri/cool/master/license.txt)
[![Github Releases](https://img.shields.io/github/release/verri/cool.svg)](https://github.com/verri/cool/releases)
[![CI](https://github.com/verri/cool/workflows/CI/badge.svg)](https://github.com/verri/cool/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/verri/cool/branch/master/graph/badge.svg)](https://codecov.io/gh/verri/cool)

Common standalone C++11 utilities.

`Cool` is a collection of self-contained headers that provide convenient
utilities missing in the standard library.

Unlike [Boost](http://www.boost.org), `Cool`'s utilities are not meant to
be "usable across a broad spectrum of applications".  Instead, each
header brings a zero-configuration solution to common problems.  As a result,
`Cool` is not as configurable as Boost, however, it intends to be much easier
to use.

Each header file contains a utility:
- [cool/ccreate.hpp](https://github.com/verri/cool/blob/master/include/cool/ccreate.hpp):
    wrapper to deal with legacy C data types that need to be created and destroyed.
- [cool/colony.hpp](https://github.com/verri/cool/blob/master/include/cool/colony.hpp):
    simplified and didactic version of std::colony.
- [cool/channel.hpp](https://github.com/verri/cool/blob/master/include/cool/channel.hpp):
    [Go-like](https://gobyexample.com/channels) channels.
- [cool/compose.hpp](https://github.com/verri/cool/blob/master/include/cool/compose.hpp):
    lambda composition (C++17 and above only).
- [cool/defer.hpp](https://github.com/verri/cool/blob/master/include/cool/defer.hpp):
    deferred execution of statements.
- [cool/enum_map.hpp](https://github.com/verri/cool/blob/master/include/cool/enum_map.hpp):
    enumeration map (C++17 and above only).
- [cool/indices.hpp](https://github.com/verri/cool/blob/master/include/cool/indices.hpp):
    utility to provide safer for loops.
- [cool/thread_pool.hpp](https://github.com/verri/cool/blob/master/include/cool/thread_pool.hpp):
    Pool of threads with queueable jobs.
- [cool/progress.hpp](https://github.com/verri/cool/blob/master/include/cool/progress.hpp):
    Progress tracking utility.

# Installation

## Recommended: Local Git Submodule

The recommended installation of the library is including it as a submodule of your Git project.

In your project, do
```
$ git submodule add https://github.com/verri/cool.git external/cool
```

## Local copy

Download the [latest release](https://github.com/verri/cool/releases) and unzip it.

## System-wide installation

*System-wide installation is recommended only if you are sure about what you doing.*

Clone the repository.
```
$ git clone https://github.com/verri/cool.git
$ cd cool
```

Install via CMake 3.11+.
```
$ cmake -S. -Bbuild
$ cmake --build build
$ cmake --install build
```

# Usage and Compilation

`Cool` is a header-only library, so you just need to instruct the compiler its location.
Some headers, however, use threads, e.g. `channel` and `thread_pool`.
Thus, you will probably need to link a thread library as well.

## Recommended: CMake 3.11+

If you installed the library as a submodule, include the following line in your `CMakeLists.txt`
```
add_subdirectory(external/cool)
find_package(Threads REQUIRED)
target_link_libraries(my-target PRIVATE cool ${CMAKE_THREAD_LIBS_INIT})
```

If you installed system-wide, use
```
find_package(Cool REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(my-target PRIVATE ${Cool_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
```

## Compiler flags

You can directly specify the compiler flags:
```
$ $CXX -I/path/to/cool/include -std=c++11 -pthreads ...
```

# Documentation

An incomplete API reference is available [here](https://verri.github.io/cool/).

Given the simplicity of the libraries, usage examples should suffice.
- [cool::ccreate](https://github.com/verri/cool/blob/master/test/ccreate.cpp)
- [cool::colony](https://github.com/verri/cool/blob/master/test/colony.cpp)
- [cool::channel](https://github.com/verri/cool/blob/master/test/channel.cpp)
- [cool::compose](https://github.com/verri/cool/blob/master/test/compose.cpp)
- [cool::defer](https://github.com/verri/cool/blob/master/test/defer.cpp)
- [cool::enum_map](https://github.com/verri/cool/blob/master/test/enum_map.cpp)
- [cool::indices](https://github.com/verri/cool/blob/master/test/indices.cpp)
- [cool::thread_pool](https://github.com/verri/cool/blob/master/test/thread_pool.cpp)
- [cool::progress](https://github.com/verri/cool/blob/master/test/progress.cpp)

# Acknowledgements

This project was supported by **FAPESP** as part of the project *"High level data
classification based on complex network applied to invariant pattern recognition"*
(Grant number 2013/25876-6).
