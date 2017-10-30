cmake_minimum_required (VERSION 3.2)

# Setup project name
project (phobos)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})

# Setup compilation flags
set(CMAKE_CXX_FLAGS "-std=c++11 -mcmodel=kernel -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostartfiles -ffreestanding -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

# Make subprojects
add_subdirectory("arch")
add_subdirectory("middlespace")
add_subdirectory("kstd")
add_subdirectory("logging")
add_subdirectory("memory")
add_subdirectory("hardware")
add_subdirectory("drivers")
add_subdirectory("filesystem")
add_subdirectory("multitasking")
add_subdirectory("cpuexceptions")
add_subdirectory("syscalls")
add_subdirectory("utils")
add_subdirectory("_demos")

# Make kernel
set(ARCH "x86_64")
set(PROJECT_ARCH "${PROJECT_NAME}-${ARCH}")
set(hdd "build/hdd.vdi")
add_executable(${PROJECT_ARCH} "main.cpp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/linker.ld")
target_link_libraries(${PROJECT_ARCH}
    "arch"
    "middlespace"
    "kstd"
    "logging"
    "memory"
    "hardware"
    "drivers"
    "filesystem"
    "multitasking"
    "cpuexceptions"
    "syscalls"
    "utils"
    "_demos"
)