file(GLOB SOURCES "*.cpp") 
add_library(logging STATIC ${SOURCES})
target_include_directories(logging PUBLIC .)    
target_link_libraries(logging kstd)