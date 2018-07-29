cmake_minimum_required (VERSION 3.10) # 3.10 for gtest_discover_tests

include(GoogleTest)

add_executable(example example.cpp)
target_link_libraries(example gtest_main gmock_main)
gtest_discover_tests(example)