file(GLOB SOURCES "*.cpp") 
add_library(time STATIC ${SOURCES})
target_include_directories(time PUBLIC .)
target_link_libraries(time kstd)