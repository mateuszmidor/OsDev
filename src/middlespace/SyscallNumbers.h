/**
 *   @file: SyscallNumbers.h
 *
 *   @date: Oct 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MIDDLESPACE_SYSCALLNUMBERS_H_
#define SRC_MIDDLESPACE_SYSCALLNUMBERS_H_

namespace middlespace {

/**
 * @brief   This enum defines system call numbers(asm: syscall) same as for Linux
 * @see     http://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/
 */
enum SyscallNumbers {
    FILE_READ               = 0,
    FILE_WRITE              = 1,
    FILE_OPEN               = 2,
    FILE_CLOSE              = 3
};

} /* namespace middlespace */

#endif /* SRC_MIDDLESPACE_SYSCALLNUMBERS_H_ */
