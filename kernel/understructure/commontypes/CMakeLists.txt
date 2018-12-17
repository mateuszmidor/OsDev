file(GLOB SOURCES "*.cpp") 
add_library(common STATIC ${SOURCES})
target_include_directories(common PUBLIC .)
target_link_libraries(common kstd)