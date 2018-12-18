file(GLOB SOURCES "*.cpp") 
add_library(abstractfilesystem STATIC ${SOURCES})
target_include_directories(abstractfilesystem PUBLIC .)
target_link_libraries(abstractfilesystem kstd)