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
enum class SysCallNumbers {
    // POSIX
    FILE_READ               = 0,
    FILE_WRITE              = 1,
    FILE_OPEN               = 2,
    FILE_CLOSE              = 3,
    FILE_STAT               = 4,
    FILE_SEEK               = 8,
    NANOSLEEP               = 35,
    FILE_TRUNCATE           = 76,
    FILE_RENAME             = 82,
    FILE_MKDIR              = 83,
    FILE_RMDIR              = 84,
    FILE_CREAT              = 85,
    FILE_UNLINK             = 87,

//    FILE_GETDENTS           = 0xdc, //220
    EXIT                    = 60,
    GET_CWD                 = 79,
    CHDIR                   = 80,
    EXIT_GROUP              = 231,

    // NON-POSIX
    VGA_CURSOR_SETVISIBLE   = 500,
    VGA_CURSOR_SETPOS       = 501,
    VGA_SET_CHAR_AT         = 502,
    VGA_FLUSH_CHAR_BUFFER   = 503,
    VGA_GET_WIDTH_HEIGHT    = 504,
    VGA_ENTER_GRAPHICS_MODE = 505,
    VGA_EXIT_GRAPHICS_MODE  = 506,
    VGA_SET_PIXEL_AT        = 507,

    FILE_ENUMERATE          = 600,
    ELF_RUN                 = 700,
    TASK_WAIT               = 800,
    TASK_LIGHTWEIGHT_RUN    = 701
};

/**
 * @brief   This enum defines legacy system call numbers(asm: int 0x80) same as for Linux
 * @see     http://asm.sourceforge.net/syscall.html
 */
enum class Int80hSysCallNumbers {
    EXIT                    = 1,
    NANOSLEEP               = 162,
    EXIT_GROUP              = 252,
};

} /* namespace middlespace */

#endif /* SRC_MIDDLESPACE_SYSCALLNUMBERS_H_ */
