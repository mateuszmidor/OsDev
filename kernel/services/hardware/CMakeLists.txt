file(GLOB SOURCES "*.cpp") 
add_library(hardware STATIC ${SOURCES})
target_include_directories(hardware PUBLIC .)    
target_link_libraries(hardware kstd)