file(GLOB SOURCES "*.cpp" "filesystem/*.cpp" "ramfs/*.cpp") 
add_library(filesystem STATIC ${SOURCES})
target_include_directories(filesystem PUBLIC . ramfs)
target_link_libraries(filesystem abstractfilesystem hardware)