/**
 *   @file: syscalls.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_SYSCALLS_H_
#define ELFS_SYSCALLS_H_

#include <cstddef>      // size_t
#include <sys/types.h>  // ssize_t
#include "SysCallNumbers.h"
#include "FsEntry.h"

namespace syscalls {

using syscall_res = unsigned long long;
using syscall_arg = unsigned long long;

inline syscall_res syscall(unsigned int syscall_no,
        syscall_arg arg1 = 0,
        syscall_arg arg2 = 0,
        syscall_arg arg3 = 0,
        syscall_arg arg4 = 0,
        syscall_arg arg5 = 0) {

    syscall_res result;
    asm volatile(
            "mov %%rbx, %%r10       ;"
            "mov %%rcx, %%r8        ;"
            "syscall                ;"
            : "=a"(result)
            : "a"(syscall_no), "D"(arg1), "S"(arg2), "d"(arg3), "b"(arg4), "c"(arg5)
    );

    return result;
}

inline ssize_t read(int fd, void* buf, size_t count) {
    return syscall(middlespace::SysCallNumbers::FILE_READ, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

inline ssize_t write(int fd, const void* buf, size_t count) {
    return syscall(middlespace::SysCallNumbers::FILE_WRITE, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

inline int open(const char* absolute_path, int flags = 0) {
    return syscall(middlespace::SysCallNumbers::FILE_OPEN, (syscall_arg)absolute_path, (syscall_arg)flags);
}

inline int close(int fd) {
    return syscall(middlespace::SysCallNumbers::FILE_CLOSE, (syscall_arg)fd);
}

inline int enumerate(unsigned int fd, middlespace::FsEntry* entries, unsigned int max_enties) {
    return syscall(middlespace::SysCallNumbers::FILE_ENUMERATE, (syscall_arg)fd, (syscall_arg)entries, (syscall_arg)max_enties);
}

inline void vga_cursor_setvisible(bool visible) {
    syscall(middlespace::SysCallNumbers::VGA_CURSOR_SETVISIBLE, (syscall_arg)visible);
}

inline void vga_cursor_setpos(unsigned int x, unsigned int y) {
    syscall(middlespace::SysCallNumbers::VGA_CURSOR_SETPOS, (syscall_arg)x, (syscall_arg)y);
}

inline void vga_setat(unsigned int x, unsigned int y, unsigned short c) {
    syscall(middlespace::SysCallNumbers::VGA_SET_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)c);
}

inline void vga_flush_buffer(const unsigned short* vga_buffer) {
    syscall(middlespace::SysCallNumbers::VGA_FLUSH_BUFFER, (syscall_arg)vga_buffer);
}

inline void vga_get_width_height(unsigned short* width, unsigned short* height) {
    syscall(middlespace::SysCallNumbers::VGA_GET_WIDTH_HEIGHT, (syscall_arg)width, (syscall_arg)height);
}

inline s64 elf_run(const char absolute_filename[], const char* nullterm_argv[]) {
    return syscall(middlespace::SysCallNumbers::ELF_RUN, (syscall_arg)absolute_filename, (syscall_arg)nullterm_argv);
}

inline int usleep(unsigned int usec) {
    asm volatile("mov $0, %rax; int $0x80");    // yield, change when blocking syscalls and timers are implemented
    return 0;
}

} // namespace syscalls



#endif /* ELFS_SYSCALLS_H_ */
