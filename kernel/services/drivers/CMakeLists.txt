file(GLOB SOURCES "*.cpp") 
add_library(drivers STATIC ${SOURCES})
target_include_directories(drivers PUBLIC .)    
target_link_libraries(drivers kstd hardware logging)