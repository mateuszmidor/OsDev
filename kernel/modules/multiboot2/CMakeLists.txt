file(GLOB SOURCES "*.cpp") 
add_library(multiboot2 STATIC ${SOURCES})
target_include_directories(multiboot2 PUBLIC .)
target_link_libraries(multiboot2 kstd elf64)