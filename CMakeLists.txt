cmake_minimum_required (VERSION 3.2)

# Setup project name
project (phobos)

# Setup global constants
set(ARCH "x86_64")
set(HDD "hdd.vdi")
set(BIN "${PROJECT_NAME}-${ARCH}.bin")
set(ISO "${PROJECT_NAME}-${ARCH}.iso")
set(GRUB_CFG "${PROJECT_SOURCE_DIR}/kernel/arch/${ARCH}/grub.cfg")

# Build kernel binary
add_subdirectory("kernel")

# Build user binaries
add_subdirectory("user")


# "make iso" - create bootable kernel iso image
add_custom_target(iso
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND cmake -DCMAKE_BUILD_TYPE=Debug ..
    COMMAND make
    COMMAND /usr/bin/env mkdir -p "isofiles/boot/grub"
    COMMAND /usr/bin/env cp kernel/${BIN} "isofiles/boot/kernel.bin"
    COMMAND /usr/bin/env cp ${GRUB_CFG} "isofiles/boot/grub"
    COMMAND grub-mkrescue -o ${ISO} "isofiles"
    COMMAND /usr/bin/env rm -r "isofiles"
)

# "make hdd" - install user programs under /BIN in HDD; "sudo remount_hdd.sh" needs to be run first
add_custom_target(hdd
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND echo "!!! REMEMBER TO FIRST: sudo remount_hdd.sh !!!"
    COMMAND make install    # install user binaries under /BIN in HDD
    COMMAND sync
)

# "make run" - just run the kernel under qemu
add_custom_target(run
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -d int -no-reboot # pcnet is AMD am79c973 network chip
)

# "make rungdb" - run the kernel under qemu, accept gdb connection at localhost:1234
add_custom_target(rungdb
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -s -S -no-reboot # pcnet is AMD am79c973 network chip
)

# "make cleanup" - remove all the files under build except for hdd
add_custom_target(cleanup
    COMMAND /usr/bin/env ls ../build | grep -v ${HDD} | xargs rm -rf
)
