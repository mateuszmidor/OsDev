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

syscall_res syscall(unsigned int syscall_no,
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

ssize_t read(int fd, void* buf, size_t count) {
    return syscall(middlespace::SyscallNumbers::FILE_READ, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return syscall(middlespace::SyscallNumbers::FILE_WRITE, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

int open(const char* absolute_path, int flags = 0) {
    return syscall(middlespace::SyscallNumbers::FILE_OPEN, (syscall_arg)absolute_path, (syscall_arg)flags);
}

int close(int fd) {
    return syscall(middlespace::SyscallNumbers::FILE_CLOSE, (syscall_arg)fd);
}

} // namespace syscalls



#endif /* ELFS_SYSCALLS_H_ */
