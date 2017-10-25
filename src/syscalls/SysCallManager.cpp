/**
 *   @file: SysCallManager.cpp
 *
 *   @date: Sep 13, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"
#include "KernelLog.h"
#include "SysCallManager.h"
#include "SysCallHandler.h"
#include "SysCallNumbers.h"
#include "middlespace/posix/posix.h"

using namespace utils;
using namespace middlespace;
namespace syscalls {

/**
 * @brief   Raw syscall handler that:
 *          1. saves user task context
 *          2. switches to kernel stack
 *          3. calls on_syscall
 *          4. switches back to user stack
 *          5. restores user task context
 *          implemented in syscalls.S
 */
extern "C" void handle_syscall();


/**
 * @brief   "syscall" handler. This is called from syscalls.S
 * @note    This is run in ring0, using kernel stack, but stays in the calling task memory space;
 *          thus can access both user memory and kernel memory (as kernel memory is mapped in each end every memory space)
 * @see     http://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/
 */
SysCallHandler syscall_handler;
extern "C" s64 on_syscall(u64 sys_call_num, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)  {
    SysCallManager& mngr = SysCallManager::instance();
    KernelLog& klog = KernelLog::instance();

//    klog.format("syscall_handler: % \n", sys_call_num);

    switch (sys_call_num) {
    case SysCallNumbers::FILE_READ: // read (unsigned int fd char *buf   size_t count)
        return syscall_handler.sys_read(arg1, (char*)arg2, arg3);

    case SysCallNumbers::FILE_WRITE: // write (fd, buff, count)
        return syscall_handler.sys_write(arg1, (const char*)arg2, arg3);

    case SysCallNumbers::FILE_OPEN: // open(const char* name, int flags, int mode)
        return syscall_handler.sys_open((const char*)arg1, arg2, arg3);

    case SysCallNumbers::FILE_CLOSE: // close
        return syscall_handler.sys_close(arg1);

    case SysCallNumbers::FILE_STAT:
        return syscall_handler.sys_stat((const char*)arg1, (struct stat*)arg2);

    case SysCallNumbers::FILE_CREAT:
        return syscall_handler.sys_creat((const char*)arg1, arg2);

    case SysCallNumbers::FILE_ENUMERATE:
        return syscall_handler.sys_enumerate(arg1, (FsEntry*)arg2, arg3);

    case SysCallNumbers::VGA_CURSOR_SETVISIBLE:
        syscall_handler.vga_cursor_setvisible((bool)arg1);
        return 0;

    case SysCallNumbers::VGA_CURSOR_SETPOS:
        syscall_handler.vga_cursor_setpos(arg1, arg2);
        return 0;

    case SysCallNumbers::VGA_SET_AT:
        syscall_handler.vga_setat(arg1, arg2, arg3);
        return 0;

    case SysCallNumbers::VGA_FLUSH_BUFFER:
        syscall_handler.vga_flush_buffer((const u16*)arg1);
        return 0;

    case SysCallNumbers::VGA_GET_WIDTH_HEIGHT:
        syscall_handler.vga_get_width_height((u16*)arg1, (u16*)arg2);
        return 0;

    case SysCallNumbers::ELF_RUN:   // (char[] filename, char** argv)
        return syscall_handler.elf_run((const char*)arg1, (const char**)arg2);

    case 9: // mmap
        return 10*1024*1024;

    case 12: // brk
        return 7*1024*1024;

    case 14: // sigprocmask
        return 0;

    case 20: // writev
        return  arg3; // all bytes written

    case 21: // access
        return -1;

    case 59: //  sys_execve
        return 0;

    case SysCallNumbers::EXIT:
        syscall_handler.sys_exit(arg1);
        return 0;   // never reached as the caller gets killed

    case SysCallNumbers::EXIT_GROUP:
        syscall_handler.sys_exit_group(arg1);
        return 0;   // never reached as the caller gets killed

    default:
        return 0;
    }
}

SysCallManager SysCallManager::_instance;

SysCallManager& SysCallManager::instance() {
    return _instance;
}

/**
 * @brief   Configure syscall-related Model Specific Registers and enable the "syscall/sysret" instructions in CPU
 */
void SysCallManager::config_and_activate_syscalls() {
    hardware::Gdt gdt;
    MSR_STAR s_star;
    s_star.syscall_cs_ss = gdt.get_kernel_code_segment_selector() ;
    s_star.sysret_cs_ss = gdt.get_user_data_segment_selector() - 8;
    s_star.syscall_target_eip_32bit = 0;

    u32 mask = 0x200; // disable interrupts
    u64 lstar = (u64)handle_syscall;
    u32 lstar_lo = lstar & 0xFFFFFFFF;
    u32 lstar_hi = lstar >> 32;
    u64 star;
    memcpy(&star, &s_star, sizeof(s_star));
    u32 star_lo = star & 0xFFFFFFFF;
    u32 star_hi = star >> 32;

    asm volatile (
            // write SFMASK to MSR 0xC000_0084
            "mov $0xC0000084, %%ecx     \n;"
            "mov %0, %%rax              \n"
            "wrmsr                      \n"

            // write CSTAR to MSR 0xC000_0083 - only for 32bit compatibility mode; not used in our system :)
            // "mov $0xC0000083, %%ecx     \n;"
            // "mov $0, %%eax              \n"
            // "mov %0, %%edx              \n"
            // "wrmsr                      \n"

            // write LSTAR to MSR 0xC000_0082
            "mov $0xC0000082, %%ecx     \n;"
            "mov %1, %%eax              \n"     // lstar_lo
            "mov %2, %%edx              \n"     // lstar_hi
            "wrmsr                      \n"

            // write STAR to MSR 0xC000_0081
            "mov $0xC0000081, %%ecx     \n;"
            "mov %3, %%eax              \n"     // star_lo
            "mov %4, %%edx              \n"     // star_hi
            "wrmsr                      \n"

            // enable syscall instruction in EFER Model Specific Register, otherwise Invalid Opcode exception 0x6 would be raised on syscall
            "mov $0xC0000080, %%ecx     \n;"
            "rdmsr                      \n;"
            "or $1, %%eax               \n"
            "wrmsr                      \n"
            :
            : "m"(mask), "m"(lstar_lo), "m"(lstar_hi), "m"(star_lo), "m"(star_hi)
            : "memory", "%rax", "%rcx", "%rdx"
    );
}
} /* namespace syscalls */
