file(GLOB SOURCES "*.cpp" "mbr/*.cpp" "adapters/*.cpp") 
add_library(fat32 STATIC ${SOURCES})
target_include_directories(fat32 PUBLIC . mbr adapters)
target_link_libraries(fat32 kstd drivers abstractfilesystem)