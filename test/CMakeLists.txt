cmake_minimum_required (VERSION 3.10) # 3.10 for gtest_discover_tests
project (test)

set(CMAKE_CXX_FLAGS "-std=c++11")

# download google test and enable ctest
include(gtest.cmake)

# build kernel libraries
add_subdirectory(../middlespace middlespace)
add_subdirectory(../kernel/services/kstd kstd)
add_subdirectory(../kernel/services/logging logging)
add_subdirectory(../kernel/services/hardware hardware)
add_subdirectory(../kernel/services/filesystem filesystem)

add_library(kernel_test INTERFACE)
target_link_libraries(kernel_test INTERFACE kstd hardware logging filesystem)

# build actual tests
add_subdirectory("filesystem_test")