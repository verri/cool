cmake_minimum_required(VERSION 4.0)
project(Cool VERSION 1.0.0)

configure_file("include/cool/version.hpp.in" "include/cool/version.hpp")

add_library(cool INTERFACE)

target_compile_features(cool INTERFACE cxx_std_11)

set(cool_header_files
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/defer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/ccreate.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/colony.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/compose.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/thread_pool.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/channel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/indices.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/progress.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cool/enum_map.hpp)

# target_sources(cool INTERFACE
#   $<BUILD_INTERFACE:${cool_header_files}>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cool/version.hpp>)

target_include_directories(cool INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>)

target_include_directories(cool SYSTEM INTERFACE
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

option(COOL_BUILD_TEST "whether or not to build the test" OFF)
if(COOL_BUILD_TEST)
  enable_testing()
  add_subdirectory(test)
endif()

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cool-config.cmake "
include(\${CMAKE_CURRENT_LIST_DIR}/cool-targets.cmake)
set(Cool_LIBRARY cool)
set(Cool_LIBRARIES cool)
")

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cool-config-version.cmake
  VERSION ${COOL_VERSION}
  COMPATIBILITY ExactVersion
)

install(TARGETS cool EXPORT cool-targets DESTINATION lib)

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")

install(EXPORT cool-targets DESTINATION lib/cmake/cool)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cool-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cool-config-version.cmake
  DESTINATION lib/cmake/cool)
