cmake_minimum_required(VERSION 3.2)
project(arch ASM)

file(GLOB SOURCES "x86_64/*.S")
add_library(${PROJECT_NAME} STATIC ${SOURCES})