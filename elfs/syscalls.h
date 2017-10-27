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
#include "posix/posix.h"

namespace syscalls {

using syscall_res = unsigned long long;
using syscall_arg = unsigned long long;



/**
 * @note    The below syscalls are received in SysCallManager and passed for handling to SysCallHandler
 */
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

/**
 * @brief   Change current position in file
 * @param   whence: SEEK_SET(new pos = 0 + offset), SEEK_CUR(new pos = current pos + offset), SEEK_END(new pos = file end + offset)
 * @return  New current position (in bytes), or -1 on error
 */
inline off_t lseek(int fd, off_t offset, int whence) {
    return syscall(middlespace::SysCallNumbers::FILE_SEEK,(syscall_arg)fd, (syscall_arg)offset, (syscall_arg)whence);
}

/**
 * @brief   Get virtual file system entry info
 * @return  0 on success, -1 on no such file
 */
inline int stat(const char path[], struct stat* statbuf) {
    return syscall(middlespace::SysCallNumbers::FILE_STAT, (syscall_arg)path, (syscall_arg)statbuf);
}

/**
 * @brief   Rename file/directory
 * @return  0 on success, negative error code on error
 */
inline int rename(const char oldname[], const char newname[]) {
    return syscall(middlespace::SysCallNumbers::FILE_RENAME, (syscall_arg)oldname, (syscall_arg)newname);
}

/**
 * @brief   Create directory
 * @return  0 on success, negative error code on error
 * @note    "mode" is not used now
 */
inline int mkdir(const char path[], int mode = 0) {
    return syscall(middlespace::SysCallNumbers::FILE_MKDIR, (syscall_arg)path, (syscall_arg)mode);
}

/**
 * @brief   Remove directory
 * @return  0 on success, negative error code on error
 * @note    Directory must  be empty
 */
inline int rmdir(const char path[]) {
    return syscall(middlespace::SysCallNumbers::FILE_RMDIR, (syscall_arg)path);
}

inline int creat(const char path[], int mode = 0) {
    return syscall(middlespace::SysCallNumbers::FILE_CREAT, (syscall_arg)path, (syscall_arg)mode);
}

/**
 * @brief   Get current working directory
 * @param   buff Buffer for storing the cwd
 * @param   size Length of the "buff", should be at least 256
 * @return  "buff" pointer itself on success, nullptr on failure
 */
inline char* getcwd(char buff[], size_t size) {
    return (char*)syscall(middlespace::SysCallNumbers::GET_CWD, (syscall_arg)buff, (syscall_arg)size);
}

/**
 * @brief   Change current working directory
 * @return  0 on success, -1 on error
 */
inline int chdir(const char path[]) {
    return syscall(middlespace::SysCallNumbers::CHDIR, (syscall_arg)path);
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
