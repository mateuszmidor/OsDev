cmake_minimum_required (VERSION 3.2)

# Setup project name and used languages
project(kernel CXX ASM)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/services/arch/${ARCH}/linker.ld")

# Setup compilation and linking flags
set(CMAKE_CXX_FLAGS "-std=c++11 -static -mcmodel=kernel -fno-stack-protector -fno-pic -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostdlib -ffreestanding")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g3")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -n -T ${LINKER_SCRIPT}")

# Collect source directories
set(SOURCE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/services/arch/${ARCH}"
    "${PROJECT_SOURCE_DIR}/../middlespace"
    "${PROJECT_SOURCE_DIR}/../middlespace/cstd"
    "${PROJECT_SOURCE_DIR}/services/cpuexceptions"
    "${PROJECT_SOURCE_DIR}/services/drivers"
    "${PROJECT_SOURCE_DIR}/services/filesystem"
    "${PROJECT_SOURCE_DIR}/services/filesystem/base"
    "${PROJECT_SOURCE_DIR}/services/filesystem/adapters"
    "${PROJECT_SOURCE_DIR}/services/filesystem/fat32"
    "${PROJECT_SOURCE_DIR}/services/filesystem/ramfs"
    "${PROJECT_SOURCE_DIR}/services/hardware"
    "${PROJECT_SOURCE_DIR}/services/kstd"
    "${PROJECT_SOURCE_DIR}/services/logging"
    "${PROJECT_SOURCE_DIR}/services/memory"
    "${PROJECT_SOURCE_DIR}/services/multitasking"
    "${PROJECT_SOURCE_DIR}/services/time"
    "${PROJECT_SOURCE_DIR}/services/utils"
    "${PROJECT_SOURCE_DIR}/interface/syscalls"
    "${PROJECT_SOURCE_DIR}/interface/procfs"
    "${PROJECT_SOURCE_DIR}"
)

# Collect source files
foreach (DIR ${SOURCE_DIRECTORIES})
    file(GLOB ASMS "${DIR}/*.S") 
    file(GLOB CPPS "${DIR}/*.cpp") 
    list(APPEND SOURCES ${CPPS} ${ASMS})
endforeach()
    
# Create the kernel binary
add_executable(${BIN} "${SOURCES}" "main.cpp")
foreach (DIR ${SOURCE_DIRECTORIES})
    target_include_directories(${BIN} PRIVATE ${DIR})    
endforeach()
