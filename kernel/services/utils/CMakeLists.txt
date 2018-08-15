file(GLOB SOURCES "*.cpp") 
add_library(utils STATIC ${SOURCES})
target_include_directories(utils PUBLIC .)
target_link_libraries(utils kstd logging drivers)