cmake_minimum_required (VERSION 3.2)

# Setup project name and used languages
project (phobos CXX ASM)

# Setup project version
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)
set(PROJECT_PATCH_LEVEL 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL})

# Setup global constants
set(ARCH "x86_64")
set(HDD "hdd.vdi")
set(BIN "${PROJECT_NAME}-${ARCH}.bin")
set(ISO "${PROJECT_NAME}-${ARCH}.iso")
set(GRUB_CFG "${PROJECT_SOURCE_DIR}/src/arch/${ARCH}/grub.cfg")
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/src/arch/${ARCH}/linker.ld")

# Setup compilation and linking flags
set(CMAKE_CXX_FLAGS "-std=c++11 -static -mcmodel=kernel -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -nostartfiles -ffreestanding -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -n -T ${LINKER_SCRIPT}")

# Collect source directories
set(SOURCE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/src/_demos"
    "${PROJECT_SOURCE_DIR}/src/arch/${ARCH}"
    "${PROJECT_SOURCE_DIR}/src/cpuexceptions"
    "${PROJECT_SOURCE_DIR}/src/drivers"
    "${PROJECT_SOURCE_DIR}/src/filesystem"
    "${PROJECT_SOURCE_DIR}/src/filesystem/adapters"
    "${PROJECT_SOURCE_DIR}/src/filesystem/fat32"
    "${PROJECT_SOURCE_DIR}/src/filesystem/procfs"
    "${PROJECT_SOURCE_DIR}/src/filesystem/ramfs"
    "${PROJECT_SOURCE_DIR}/src/hardware"
    "${PROJECT_SOURCE_DIR}/src/kstd"
    "${PROJECT_SOURCE_DIR}/src/logging"
    "${PROJECT_SOURCE_DIR}/src/memory"
    "${PROJECT_SOURCE_DIR}/src/middlespace"
#    "${PROJECT_SOURCE_DIR}/src/middlespace/posix"
    "${PROJECT_SOURCE_DIR}/src/multitasking"
    "${PROJECT_SOURCE_DIR}/src/syscalls"
    "${PROJECT_SOURCE_DIR}/src/utils"
)

# Collect source files
foreach (DIR ${SOURCE_DIRECTORIES})
    file(GLOB ASMS "${DIR}/*.S") 
    file(GLOB CPPS "${DIR}/*.cpp") 
    list(APPEND SOURCES ${CPPS} ${ASMS})
endforeach()
    
# Create the kernel binary
add_executable(${BIN} "${SOURCES}" "src/main.cpp")
foreach (DIR ${SOURCE_DIRECTORIES})
    message(STATUS "[${DIR}]")
    target_include_directories(${BIN} PRIVATE ${DIR})    
endforeach()

# "make iso"
add_custom_target(iso
    COMMAND make
    COMMAND mkdir -p "isofiles/boot/grub"
    COMMAND cp ${BIN} "isofiles/boot/kernel.bin"
    COMMAND cp ${GRUB_CFG} "isofiles/boot/grub"
    COMMAND grub-mkrescue -o ${ISO} "isofiles"
    COMMAND rm -r "isofiles"
)
    
# "make run"
add_custom_target(run
    COMMAND make iso
    COMMAND cp "${PROJECT_SOURCE_DIR}/media/hdd.vdi" ${HDD}
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -d int -no-reboot # pcnet is AMD am79c973 network chip
)

# "make rungdb"
add_custom_target(rungdb
    COMMAND make iso
    COMMAND cp "${PROJECT_SOURCE_DIR}/media/hdd.vdi" ${HDD}
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -s -S -no-reboot # pcnet is AMD am79c973 network chip
)
