file(GLOB SOURCES "*.cpp") 
add_library(elf64 STATIC ${SOURCES})
target_include_directories(elf64 PUBLIC .)
target_link_libraries(elf64 kstd memory multitasking)