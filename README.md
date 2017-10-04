# PhobOS
This is a study project for operating systems development for x86.
Based on:  
 + http://wyoos.org  
 + http://wiki.osdev.org
 + http://www.jamesmolloy.co.uk/tutorial_html  
 + http://os.phil-opp.com/multiboot-kernel.html  
 + https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf  
 
Credits to the authors!  
    
# Features
 + GRUB2 bootable
 + 64 bit kernel
 + higher-half (linux-like, kernel starts at virt. 0xFFFFFFFF80000000)
 + multitasking
 + kernel/user space
 + dynamic memory (simple bump allocator for now)
 + memory paging (identity-mapped low 1 GB for now)
 + MBR/Fat32 driver (no long names)
 + PS/2 mouse driver
 + Keyboard driver
 + VGA driver (90x30 text mode, 320x200x8bit graphics mode for now)
 
# Need install
 + xorriso
 + qemu-system-x86_64

# Run it
set your emulator to network chip AMD am79c973 (default for VBox, for QEMU add in command line: -net nic,model=pcnet)
> make run

# Debug it in terminal
> make rungdb  
> gdb -symbols=build/kernel-x86_64.bin -ex "set arch i386:x86-64:intel" -ex "target remote localhost:1234"  
(gdb) break kmain  
(gdb) continue  
(gdb) ^a + ^x  

# Debug in in Eclipse
Run->Debug Configurations... ->C/C++ Remote Application->add  
Set Project to PhobOS  
Set "Using GDB(DSF) Manual Remote Debugging Launcher" instead of Automatic one  
Under Debugger->Connection tab set:
 + Stop on startup at: kmain  
 + Port number to 1234  
 
(terminal) make rungdb  
(eclipse) Debug->our newly created configuration  

# GDB pretty printing of STL containers
 + Install python3
 + Create .gdbinit in project folder with following contents:  
python  
import sys  
sys.path.insert(0, '/home/mateusz/gdb_printers/python')  
from libstdcxx.v6.printers import register_libstdcxx_printers  
register_libstdcxx_printers (None)  
end  

Download contents of https://gcc.gnu.org/svn/gcc/trunk/libstdc++-v3/python/libstdcxx/ under:  
/home/mateusz/gdb_printers/python/libstdcxx  

Should work now.

# Tools
 + make hdd && sudo ./remount_hdd.sh - mount build/hdd.vdi partitions as "p1" and "p2" in current directory. Requires qemu-nbd
 + objdump -f - entry point logical address
 + objdump -h - elf headers
 + grub-file --is-x86-multiboot2 build/kernel-x86_64.bin; echo $? - check if kernel is multiboot2 compliant, (0 means yes)