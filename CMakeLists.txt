cmake_minimum_required (VERSION 3.2)

# Setup project name
project (phobos)

# Setup global constants
set(ARCH "x86_64")
set(HDD "hdd.vdi")
set(BIN "${PROJECT_NAME}-${ARCH}.bin")
set(ISO "${PROJECT_NAME}-${ARCH}.iso")
set(GRUB_CFG "${PROJECT_SOURCE_DIR}/kernel/arch/${ARCH}/grub.cfg")
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/kernel/arch/${ARCH}/linker.ld")

# Build kernel binary
add_subdirectory("kernel")

# Build user binaries
add_subdirectory("user")

# "make iso"
add_custom_target(iso
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND make
    COMMAND /bin/mkdir -p "isofiles/boot/grub"
    COMMAND /bin/cp kernel/${BIN} "isofiles/boot/kernel.bin"
    COMMAND /bin/cp ${GRUB_CFG} "isofiles/boot/grub"
    COMMAND grub-mkrescue -o ${ISO} "isofiles"
    COMMAND /bin/rm -r "isofiles"
)

# "make run"
add_custom_target(run
    COMMAND make iso
    COMMAND /bin/cp -n "${PROJECT_SOURCE_DIR}/media/hdd.vdi" ${HDD}
    COMMAND make install    # install user binaries in hdd
    COMMAND sync
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -d int -no-reboot # pcnet is AMD am79c973 network chip
)

# "make rungdb"
add_custom_target(rungdb
    COMMAND make iso
    COMMAND /bin/cp -n "${PROJECT_SOURCE_DIR}/media/hdd.vdi" ${HDD}
    COMMAND make install    # install user binaries in hdd
    COMMAND sync
    COMMAND qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb ${HDD} -cdrom ${ISO} -s -S -no-reboot # pcnet is AMD am79c973 network chip
)
