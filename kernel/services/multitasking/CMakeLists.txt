file(GLOB SOURCES "*.cpp") 
add_library(multitasking STATIC ${SOURCES})
target_include_directories(multitasking PUBLIC .)
target_link_libraries(multitasking kstd hardware memory filesystem)