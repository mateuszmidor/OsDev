/**
 *   @file: syscalls.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_SYSCALLS_H_
#define ELFS_SYSCALLS_H_

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

int write(int fd, const char str[], unsigned int len) {
    return syscall(1, (syscall_arg)fd, syscall_arg(str), syscall_arg(len));
}



#endif /* ELFS_SYSCALLS_H_ */
