cmake_minimum_required(VERSION 3.2)
project(_demos)

file(GLOB SOURCES "*.cpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} "kstd")
target_link_libraries(${PROJECT_NAME} "logging")
target_link_libraries(${PROJECT_NAME} "multitasking")
target_include_directories(${PROJECT_NAME} PUBLIC ".")