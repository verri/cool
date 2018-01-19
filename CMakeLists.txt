cmake_minimum_required(VERSION 3.0)
project(Cool VERSION 0.0.0)

configure_file("include/cool/version.hpp.in" "include/cool/version.hpp")

add_library(cool INTERFACE)

target_compile_features(cool INTERFACE cxx_std_11)

set(cool_header_files
  include/cool/defer.hpp
  include/cool/ccreate.hpp
  include/cool/thread_pool.hpp
  include/cool/channel.hpp
  include/cool/indices.hpp)

target_sources(cool INTERFACE
  $<INSTALL_INTERFACE:${cool_header_files}>
  $<INSTALL_INTERFACE:include/cool/version.hpp>)

target_include_directories(cool INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>)
target_include_directories(cool SYSTEM INTERFACE $<INSTALL_INTERFACE:/include>)

option(COOL_BUILD_TEST "whether or not to build the test" OFF)
if(COOL_BUILD_TEST)
  enable_testing()
  add_subdirectory(test)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cool-config-version.cmake
  VERSION ${COOL_VERSION}
  COMPATIBILITY ExactVersion
)

install(TARGETS cool EXPORT cool-targets DESTINATION lib)
install(EXPORT cool-targets FILE cool-config.cmake DESTINATION lib/cmake/cool)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cool-config-version.cmake DESTINATION lib/cmake/cool)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")
