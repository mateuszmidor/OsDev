file(GLOB SOURCES "*.cpp") 
add_library(sysinfo STATIC ${SOURCES})
target_include_directories(sysinfo PUBLIC .)
target_link_libraries(sysinfo kstd hardware drivers)