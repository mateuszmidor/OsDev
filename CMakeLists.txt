cmake_minimum_required (VERSION 3.2)

# Setup project name
project (phobos)

option(BUILD_KERNEL "Build the kernel binary" OFF)
option(BUILD_USER "Build the user binaries" OFF)
option(BUILD_TESTS "Build the unit tests" OFF)

# Setup global constants
set(ARCH "x86_64")
set(HDD "hdd.vdi")
set(BIN "${PROJECT_NAME}-${ARCH}.bin")
set(ISO "${PROJECT_NAME}-${ARCH}.iso")
set(GRUB_CFG "${PROJECT_SOURCE_DIR}/kernel/arch/${ARCH}/grub.cfg")

# Build kernel binary if requested
if (BUILD_KERNEL)
    add_subdirectory("kernel")
    
    # "make iso" - create bootable kernel iso image
    add_custom_target(iso
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND /usr/bin/env mkdir -p "isofiles/boot/grub"
        COMMAND /usr/bin/env cp ${GRUB_CFG} "isofiles/boot/grub"
        COMMAND /usr/bin/env cp kernel/${BIN} "isofiles/boot/kernel.bin"
        COMMAND grub-mkrescue -o ${ISO} "isofiles"
        COMMAND /usr/bin/env rm -r "isofiles"
    )
endif()

# Build user binaries if requested
if (BUILD_USER)
    add_subdirectory("user")
    # run `make install` to install user binaries in hdd.vdi (/BIN)
endif()

# Build unit tests if requested
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()