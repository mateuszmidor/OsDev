cmake_minimum_required(VERSION 3.2)
project(ustd)

file(GLOB SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "src/" "../../middlespace")