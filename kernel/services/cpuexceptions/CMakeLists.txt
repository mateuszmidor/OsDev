file(GLOB SOURCES "*.cpp") 
add_library(cpuexceptions STATIC ${SOURCES})
target_include_directories(cpuexceptions PUBLIC .)    
target_link_libraries(cpuexceptions hardware)