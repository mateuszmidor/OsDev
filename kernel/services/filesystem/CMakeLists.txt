file(GLOB SOURCES "*.cpp" "base/*.cpp" "ramfs/*.cpp") 
add_library(filesystem STATIC ${SOURCES})
target_include_directories(filesystem PUBLIC . base ramfs)
target_link_libraries(filesystem kstd hardware logging)