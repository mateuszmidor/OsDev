# PhobOS
This is a study project for operating systems development for x86-64.
Based on:  
 + http://wyoos.org  
 + http://wiki.osdev.org
 + http://www.jamesmolloy.co.uk/tutorial_html  
 + http://os.phil-opp.com/multiboot-kernel.html  
 + http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf
 + https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf  
 
Credits to the authors!  
  
# Showcase
Booting  
![Logo](https://github.com/mateuszmidor/OsDev/blob/master/media/boot.gif)
  
Simple dir tree demo  
![Logo](https://github.com/mateuszmidor/OsDev/blob/master/media/tree.gif)
  
Simple plot demo  
![Logo](https://github.com/mateuszmidor/OsDev/blob/master/media/plot.gif)
  
Simple arkanoid demo  
![Logo](https://github.com/mateuszmidor/OsDev/blob/master/media/arkanoid.gif)
  
# Features
 + GRUB2 bootable
 + 64 bit kernel
 + memory paging (2MB pages) and on-page-fault allocator
 + stack guard page
 + managed dynamic memory
 + higher-half kernel
 + multitasking
 + kernel/user space
 + ELF64 support (statically linked only) - some basic syscalls implemented, see: syscalls.h
 + virtual filesystem 
 + MBR/Fat32 driver (no long names)
 + PS/2 mouse driver
 + Keyboard driver
 + VGA driver (90x30 text mode, 320x200 graphics mode for now)
 + Simple terminal emulator (with TAB auto-completion) and a few basic system commands like cd, cat, cp, mkdir
 
# Need install
 + cmake, at least v3.10
 + xorriso (for building bootable iso PhobOS image)
 + mtools (for fat32 formatting of PhobOS virtual drive)
 + qemu-system-x86_64, with qemu-nbd (for running PhobOS, for mounting virtual phobos drive in linux)

# Run it (tested on ubuntu 16.4 & recent manjaro)
> sudo ./remount_hdd.sh
> ./build_kernel.sh && ./build_user.sh && ./run.sh

# Debug it in terminal
> sudo ./remount_hdd.sh
> ./build_kernel.sh && ./build_user.sh && ./rungdb.sh
> gdb -symbols=build/kernel/phobos-x86_64.bin -ex "set arch i386:x86-64:intel" -ex "target remote localhost:1234"
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
 + sudo ./remount_hdd.sh - mount build/hdd.vdi partitions as p1, p2, p3, p4 in current directory. Requires qemu-nbd
 + objdump -f - entry point logical address
 + objdump -h - elf headers
 + grub-file --is-x86-multiboot2 build/kernel-x86_64.bin; echo $? - check if kernel is multiboot2 compliant, (0 means yes)
