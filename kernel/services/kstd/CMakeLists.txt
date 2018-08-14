file(GLOB SOURCES "*.cpp") 
add_library(kstd STATIC ${SOURCES})
target_include_directories(kstd PUBLIC .)
target_link_libraries(kstd cstd middlespace)  # kernel standard depends on common standard