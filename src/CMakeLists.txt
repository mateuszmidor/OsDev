cmake_minimum_required (VERSION 3.2)

# Setup project name
project (phobos)
set(ARCH "x86_64")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/arch/${ARCH}/linker.ld")

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})

# Setup compilation flags
set(CMAKE_CXX_FLAGS "-std=c++11 -static -mcmodel=kernel -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostartfiles -ffreestanding -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_ASM_FLAGS "${CMAKE_CXX_FLAGS} -x assembler-with-cpp")
SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

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

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id=none -n -T ${LINKER_SCRIPT}")
set(PROJECT_ARCH "${PROJECT_NAME}-${ARCH}.bin")
set(ISO "kernel-${ARCH}.iso")
set(HDD "hdd.vdi")

set(GRUB_CFG "${CMAKE_SOURCE_DIR}/arch/${ARCH}/grub.cfg")
add_executable(${PROJECT_ARCH} "main.cpp")

target_link_libraries(${PROJECT_ARCH}
    "-Wl,--whole-archive" "arch" "-Wl,--no-whole-archive" # multiboot2 header must be included
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

add_custom_target(run
      #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND mkdir -p "isofiles/boot/grub"
    COMMAND cp ${PROJECT_ARCH} "isofiles/boot/kernel.bin"
    COMMAND cp ${GRUB_CFG} "isofiles/boot/grub"
    COMMAND grub-mkrescue -o ${ISO} "isofiles"
    COMMAND rm -r "isofiles"
    COMMAND cp "${CMAKE_SOURCE_DIR}/../media/hdd.vdi" ${HDD}
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -d int -no-reboot # pcnet is AMD am79c973 network chip
)
