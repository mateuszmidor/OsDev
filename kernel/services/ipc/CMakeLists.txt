file(GLOB SOURCES "*.cpp") 
add_library(ipc STATIC ${SOURCES})
target_include_directories(ipc PUBLIC .)
target_link_libraries(ipc kstd filesystem)