cmake_minimum_required (VERSION 3.2)

# Setup project name and used languages
project(kernel CXX ASM)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/arch/${ARCH}/linker.ld")

# Setup compilation and linking flags
set(CMAKE_CXX_FLAGS "-std=c++11 -static -mcmodel=kernel -fno-stack-protector -fno-pic -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostdlib -ffreestanding")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g3")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -n -T ${LINKER_SCRIPT}")


# Add subprojects
add_subdirectory(../middlespace middlespace)
add_subdirectory(understructure)
add_subdirectory(services)
add_subdirectory(modules)
add_subdirectory(interface)


# Collect source files
file(GLOB SOURCES_ASM "arch/${ARCH}/*.S") 
file(GLOB SOURCES_CPP "*.cpp") 
    
# Create the kernel binary
add_executable(${BIN} ${SOURCES_ASM} ${SOURCES_CPP})
#target_include_directories(${BIN} .)    

target_link_libraries(${BIN}
    kstd hardware logging memory time filesystem multitasking drivers cpuexceptions ipc utils # sevices
    fat32 elf64 multiboot2 vga # modules
    int80h procfs syscalls # interface
)