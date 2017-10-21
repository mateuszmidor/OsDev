/**
 *   @file: syscalls.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_SYSCALLS_H_
#define ELFS_SYSCALLS_H_

#include "SyscallNumbers.h"
#include <cstddef>      // size_t
#include <sys/types.h>  // ssize_t

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
    return syscall(middlespace::SyscallNumbers::FILE_READ, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

inline ssize_t write(int fd, const void* buf, size_t count) {
    return syscall(middlespace::SyscallNumbers::FILE_WRITE, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

inline int open(const char* absolute_path, int flags = 0) {
    return syscall(middlespace::SyscallNumbers::FILE_OPEN, (syscall_arg)absolute_path, (syscall_arg)flags);
}

inline int close(int fd) {
    return syscall(middlespace::SyscallNumbers::FILE_CLOSE, (syscall_arg)fd);
}

inline void vga_cursor_setvisible(bool visible) {
    syscall(middlespace::SyscallNumbers::VGA_CURSOR_SETVISIBLE, (syscall_arg)visible);
}

inline void vga_cursor_setpos(unsigned int x, unsigned int y) {
    syscall(middlespace::SyscallNumbers::VGA_CURSOR_SETPOS, (syscall_arg)x, (syscall_arg)y);
}

inline void vga_setat(unsigned int x, unsigned int y, unsigned short c) {
    syscall(middlespace::SyscallNumbers::VGA_SET_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)c);
}


inline int usleep(unsigned int usec) {
    asm volatile("mov $0, %rax; int $0x80");    // yield, this is to be madeover in future
    return 0;
}

} // namespace syscalls



#endif /* ELFS_SYSCALLS_H_ */
