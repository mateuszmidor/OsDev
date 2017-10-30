cmake_minimum_required(VERSION 3.2)
project(middlespace)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ".")