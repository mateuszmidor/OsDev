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

# Debug it in terminal
make rungdb
gdb -symbols=build/kernel-x86_64.bin -ex "set arch i386:x86-64:intel" -ex "target remote localhost:1234"
(gdb) break kmain
(gdb) continue
(gdb) ^a + ^x

# Debug in in Eclipse
Run->Debug Configurations... ->C/C++ Remote Application->add
Set Project to PhobOS
Set "Using GDB(DSF) Manual Remote Debugging Launcher" instead of Automatic one
Under Debugger->Connection tab set 
    Stop on startup at: kmain 
    Port number to 1234
(terminal) make rungdb
(eclipse) Debug->our newly created configuration

# Tools
objdump -f - entry point logical address
objdump -h - elf headers
grub-file --is-x86-multiboot2 build/kernel-x86_64.bin; echo $? - check if kernel is multiboot2 compliant, (0 means yes)
make hdd && sudo ./remount_hdd.sh - mount build/hdd.vdi partitions as "p1" and "p2" in current directory