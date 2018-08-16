cmake_minimum_required (VERSION 3.2)

# Setup project name
project (user CXX)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/linker.ld")

# Setup compilation flags
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -static -mno-red-zone -fno-stack-protector -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostdlib -ffreestanding -fdata-sections -ffunction-sections -Wl,--gc-sections")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g3")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -n -T ${LINKER_SCRIPT}")

# Make the project from subprojects
add_subdirectory("../middlespace" "middlespace")
add_subdirectory("ustd") 
add_subdirectory("bin")