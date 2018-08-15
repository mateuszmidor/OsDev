file(GLOB SOURCES "*.cpp") 
add_library(procfs STATIC ${SOURCES})
target_include_directories(procfs PUBLIC .)
target_link_libraries(procfs 
    kstd hardware filesystem logging memory drivers multitasking utils
    fat32
)