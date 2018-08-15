file(GLOB SOURCES "*.cpp") 
add_library(syscalls STATIC ${SOURCES})
target_include_directories(syscalls PUBLIC .)
target_link_libraries(syscalls 
    kstd hardware drivers filesystem multitasking time ipc utils # services
    elf64 # modules
)