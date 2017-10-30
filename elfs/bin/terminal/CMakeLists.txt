cmake_minimum_required(VERSION 3.2)
project(terminal)

file(GLOB SOURCES "src/*.cpp" "src/cmds/*.cpp") 
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE "ustd")
target_include_directories(${PROJECT_NAME} PRIVATE "src/" "src/cmds/")
get_filename_component(BIN "./../../../p1" ABSOLUTE)
install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN})