cmake_minimum_required (VERSION 3.2)

# Setup project name
project (user CXX)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})

# Setup compilation flags
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -static -mno-red-zone -nostdlib -fno-use-cxa-atexit -fno-rtti -fno-exceptions -ffreestanding -fdata-sections -ffunction-sections -Wl,--gc-sections")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

# Make the project from subprojects
add_subdirectory("ustd") 
add_subdirectory("bin") 