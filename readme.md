# Cool

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
- [cool/channel.hpp](https://github.com/verri/cool/blob/master/include/cool/channel.hpp): 
    [Go-like](https://gobyexample.com/channels) channels.
- [cool/defer.hpp](https://github.com/verri/cool/blob/master/include/cool/defer.hpp): 
    deferred execution of statements.
- [cool/indices.hpp](https://github.com/verri/cool/blob/master/include/cool/indices.hpp): 
    utility to provide safer for loops.
- [cool/thread_pool.hpp](https://github.com/verri/cool/blob/master/include/cool/thread_pool.hpp): 
    Pool of threads with queueable jobs.

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

Install via CMake 3.0+.
```
$ mkdir build
$ cd build
$ cmake ..
$ make
# make install
```

# Usage and Compilation

`Cool` is a header-only library, so you just need to instruct the compiler its location.
Some headers, however, use threads, e.g. `channel` and `thread_pool`.
Thus, you will probably need to link a thread library as well.

## Recommended: CMake 3.0+

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
target_link_libraries(my-target PRIVATE cool ${CMAKE_THREAD_LIBS_INIT})
```

## Compiler flags

You can directly specify the compiler flags:
```
$ $CXX -I/path/to/cool/include -std=c++11 -pthreads ...
```
