Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1
)
FetchContent_MakeAvailable(Catch2)

set(source_files
  test_suite.cpp
  ccreate.cpp
  colony.cpp
  compatibility.cpp
  defer.cpp
  channel.cpp
  progress.cpp
  thread_pool.cpp
  indices.cpp
  version.cpp)

set(COOL_TEST_STANDARD 11 CACHE STRING "C++ version to compile the tests")
if(COOL_TEST_STANDARD GREATER 14)
  set(source_files ${source_files} compose.cpp enum_map.cpp)
endif()

add_executable(cool_test_suite ${source_files})

set_target_properties(cool_test_suite PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  CXX_STANDARD ${COOL_TEST_STANDARD})

option(COOL_TEST_COVERAGE "whether or not add coverage instrumentation" OFF)
if (COOL_TEST_COVERAGE)
  target_compile_options(cool_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-O0>")
  target_compile_options(cool_test_suite PRIVATE "$<$<CONFIG:DEBUG>:--coverage>")
  set_target_properties(cool_test_suite PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
else()
  target_compile_options(cool_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-O0>")
  target_compile_options(cool_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>")
  target_compile_options(cool_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=address,leak,undefined>")
  set_target_properties(cool_test_suite PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,leak,undefined -fuse-ld=gold")
endif()

target_include_directories(cool_test_suite PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(cool_test_suite PUBLIC cool ${CMAKE_THREAD_LIBS_INIT} Catch2::Catch2WithMain)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    target_compile_options(cool_test_suite PRIVATE "/W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(cool_test_suite PRIVATE -Wall -Wextra -pedantic)
endif()

add_test(NAME test.cool COMMAND cool_test_suite)
