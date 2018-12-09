file(GLOB SOURCES "*.cpp" "filesystem/*.cpp" "ramfs/*.cpp") 
add_library(filesystem STATIC ${SOURCES})
target_include_directories(filesystem PUBLIC . ramfs PRIVATE filesystem)
target_link_libraries(filesystem kstd hardware)