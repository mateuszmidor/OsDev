cmake_minimum_required (VERSION 3.10) # 3.10 for gtest_discover_tests
project (test)

set(CMAKE_CXX_FLAGS "-std=c++11")

# download google test and enable ctest
include(gtest.cmake)

# Collect source directories
set(SOURCE_DIRECTORIES
#    "${PROJECT_SOURCE_DIR}/../kernel/services/arch/${ARCH}"
    "${PROJECT_SOURCE_DIR}/../kernel/../middlespace"
    "${PROJECT_SOURCE_DIR}/../kernel/../middlespace/cstd"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/cpuexceptions"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/drivers"
    "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/adapters"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/fat32"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/procfs"
    "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/ramfs"
#    "${PROJECT_SOURCE_DIR}/../kernel/services/hardware"
    "${PROJECT_SOURCE_DIR}/../kernel/services/kstd"
    "${PROJECT_SOURCE_DIR}/../kernel/services/logging"
    "${PROJECT_SOURCE_DIR}/../kernel/services/memory"
    "${PROJECT_SOURCE_DIR}/../kernel/services/multitasking"
    "${PROJECT_SOURCE_DIR}/../kernel/services/time"
    "${PROJECT_SOURCE_DIR}/../kernel/services/utils"
#    "${PROJECT_SOURCE_DIR}/../kernel/interface/syscalls"
#    "${PROJECT_SOURCE_DIR}/../kernel"
)

# Collect source files
#foreach (DIR ${SOURCE_DIRECTORIES})
#    file(GLOB ASMS "${DIR}/*.S") 
#    file(GLOB CPPS "${DIR}/*.cpp") 
#    list(APPEND SOURCES ${CPPS} ${ASMS})
#endforeach()
 
 set (SOURCES 
        "${PROJECT_SOURCE_DIR}/../kernel/../middlespace/cstd/Conversions.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/../middlespace/cstd/StringUtils.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/logging/KernelLog.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/UnixPath.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/VfsCachedEntry.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/EntryCache.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/VfsTree.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/ramfs/VfsRamDirectoryEntry.cpp"
        "${PROJECT_SOURCE_DIR}/../kernel/services/filesystem/ramfs/VfsRamMountPoint.cpp"
 )
 
add_library(kernel_test "${SOURCES}")
foreach (DIR ${SOURCE_DIRECTORIES})
    target_include_directories(kernel_test PUBLIC ${DIR})    
endforeach()

add_subdirectory("test")