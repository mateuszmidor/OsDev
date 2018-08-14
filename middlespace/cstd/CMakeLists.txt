file(GLOB SOURCES "*.cpp") 
add_library(cstd STATIC ${SOURCES})
target_include_directories(cstd PUBLIC .) 