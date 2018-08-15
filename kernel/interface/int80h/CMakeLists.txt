file(GLOB SOURCES "*.cpp") 
add_library(int80h STATIC ${SOURCES})
target_include_directories(int80h PUBLIC .)
target_link_libraries(int80h kstd logging multitasking drivers middlespace)