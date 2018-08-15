file(GLOB SOURCES "*.cpp") 
add_library(vga STATIC ${SOURCES})
target_include_directories(vga PUBLIC .)
target_link_libraries(vga kstd drivers)