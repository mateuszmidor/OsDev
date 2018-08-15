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


add_subdirectory(../middlespace middlespace)
add_subdirectory(../middlespace/cstd middlespace/cstd)
add_subdirectory(services/kstd)
add_subdirectory(services/hardware)
add_subdirectory(services/logging)
add_subdirectory(services/memory)
add_subdirectory(services/time)
add_subdirectory(services/filesystem)
add_subdirectory(services/multitasking)
add_subdirectory(services/drivers)
add_subdirectory(services/cpuexceptions)
add_subdirectory(services/ipc)
add_subdirectory(services/utils)
add_subdirectory(modules/fat32)
add_subdirectory(interface/procfs)
add_subdirectory(interface/syscalls)

# Collect source directories
set(SOURCE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/arch/${ARCH}"
    "${PROJECT_SOURCE_DIR}"
)

# Collect source files
foreach (DIR ${SOURCE_DIRECTORIES})
    file(GLOB ASMS "${DIR}/*.S") 
    file(GLOB CPPS "${DIR}/*.cpp") 
    list(APPEND SOURCES ${CPPS} ${ASMS})
endforeach()
    
# Create the kernel binary
add_executable(${BIN} "${SOURCES}")

foreach (DIR ${SOURCE_DIRECTORIES})
    target_include_directories(${BIN} PRIVATE ${DIR})    
endforeach()

target_link_libraries(${BIN}
    kstd middlespace hardware logging memory time filesystem multitasking drivers cpuexceptions ipc utils # sevices
    fat32  # modules
    procfs syscalls # interface
)