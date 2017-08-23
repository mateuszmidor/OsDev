# OsDev
OS development study
Based on:
    http://wyoos.org
    http://os.phil-opp.com/multiboot-kernel.html
    
    
# Need install
sudo apt install xorriso
sudo apt install qemu-system-x86


# Run it
set your emulator to network chip AMD am79c973 (default for VBox, for QEMU: -net nic,model=pcnet)
make run
make debug


# Tools
objdump -f - entry point logical address
objdump -h - elf headers
grub-file --is-x86-multiboot2 build/kernel-x86_64.bin; echo $? - check if kernel is multiboot2 compliant, (0 means yes)
make hdd && sudo ./remount_hdd.sh - mount build/hdd.vdi partitions as "p1" and "p2" in current directory