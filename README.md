# OsDev
OS development study
Based on:
    http://wyoos.org
    http://os.phil-opp.com/multiboot-kernel.html
    
# Need install
sudo apt install xorriso
sudo apt install qemu-system-x86

# Tools
objdump -f - entry point logical address
objdump -h - elf headers
grub-file --is-x86-multiboot2 build/kernel-x86_64.bin; echo $? - check if kernel is multiboot2 compliant, (0 means yes)