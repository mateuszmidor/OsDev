file(GLOB SOURCES "*.cpp" "ramfs/*.cpp") 
add_library(filesystem STATIC ${SOURCES})
target_include_directories(filesystem PUBLIC . ramfs)
target_link_libraries(filesystem kstd hardware logging)