file(GLOB SOURCES "*.cpp") 
add_library(memory STATIC ${SOURCES})
target_include_directories(memory PUBLIC .)
target_link_libraries(memory kstd)