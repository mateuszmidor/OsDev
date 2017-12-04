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
#include <errno.h>

namespace syscalls {

using syscall_res = signed long long;
using syscall_arg = unsigned long long;


/**
 * @brief   Sleep current task for at least given amount of nanoseconds
 *          It must be implemented as int80h until kernel is preemptible and syscall can be suspended
 * @note    when nsec == 0, it only reschedules CPU to another task
 */
inline int nsleep(unsigned long long nsec) {
    syscall_res result;

    asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(middlespace::Int80hSysCallNumbers::NANOSLEEP), "b"(nsec)
    );

    return result;
}

/**
 * @brief   Sleep current task for at least given amount of milliseconds
 *          It must be implemented as int80h until kernel is preemptible and syscall can be suspedned
 * @note    when msec == 0, it only reschedules CPU to another task
 */
inline int msleep(unsigned long long msec) {
    return nsleep(msec * 1000 * 1000);
}

/**
 * @brief   Reschedule CPU to another task
 */
inline void yield() {
    nsleep(0);
}

/**
 * @note    The below syscalls are received in SysCallManager and passed for handling to SysCallHandler
 */
inline syscall_res syscall(middlespace::SysCallNumbers syscall,
        syscall_arg arg1 = 0,
        syscall_arg arg2 = 0,
        syscall_arg arg3 = 0,
        syscall_arg arg4 = 0,
        syscall_arg arg5 = 0) {

    syscall_res result;

    do {
        // 1. make system call
        asm volatile(
                "mov %%rbx, %%r10       ;"
                "mov %%rcx, %%r8        ;"
                "syscall                ;"
                : "=a"(result)
                : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "b"(arg4), "c"(arg5)
        );

        // 2. reschedule if syscall would block
        if (result == -EWOULDBLOCK)
            yield();
    } while (result == -EWOULDBLOCK);

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
 * @brief   Truncate file to specified size
 * @return  0 on success, negative error code on error
 */
inline int truncate(const char path[], off_t length) {
    return syscall(middlespace::SysCallNumbers::FILE_TRUNCATE, (syscall_arg)path, (syscall_arg)length);
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
 * @note    Directory must  be empty. Removes only directories. For file use unlink
 */
inline int rmdir(const char path[]) {
    return syscall(middlespace::SysCallNumbers::FILE_RMDIR, (syscall_arg)path);
}

inline int creat(const char path[], int mode = 0) {
    return syscall(middlespace::SysCallNumbers::FILE_CREAT, (syscall_arg)path, (syscall_arg)mode);
}

/**
 * @brief   Remove file
 * @return  0 on success, negative error code on error
 * @note    Removes only files, for directory use rmdir
 */
inline int unlink(const char path[]) {
    return syscall(middlespace::SysCallNumbers::FILE_UNLINK, (syscall_arg)path);
}

/**
 * @brief   Move program break effectively changing amount of dynamic memory available to the task
 * @param   new_brk New dynamic memory high limit
 * @return  New program break on success, current program break on failure (no memory, new_brk == 0)
 */
inline size_t brk(size_t new_brk) {
    return syscall(middlespace::SysCallNumbers::BRK, (syscall_arg)new_brk);
}
/**
 * @brief   Get current working directory
 * @param   buff Buffer for storing the cwd
 * @param   size Length of the "buff", should be at least 256
 * @return  0 on success, negative error code on error
 */
inline int getcwd(char* buff, size_t size) {
    return syscall(middlespace::SysCallNumbers::GET_CWD, (syscall_arg)buff, (syscall_arg)size);
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

inline void vga_set_char_at(unsigned int x, unsigned int y, unsigned short vga_character) {
    syscall(middlespace::SysCallNumbers::VGA_SET_CHAR_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)vga_character);
}

inline void vga_flush_char_buffer(const unsigned short* text_buffer) {
    syscall(middlespace::SysCallNumbers::VGA_FLUSH_CHAR_BUFFER, (syscall_arg)text_buffer);
}

inline void vga_get_width_height(unsigned short* width, unsigned short* height) {
    syscall(middlespace::SysCallNumbers::VGA_GET_WIDTH_HEIGHT, (syscall_arg)width, (syscall_arg)height);
}

inline void vga_enter_graphics_mode() {
    syscall(middlespace::SysCallNumbers::VGA_ENTER_GRAPHICS_MODE);
}

inline void vga_exit_graphics_mode() {
    syscall(middlespace::SysCallNumbers::VGA_EXIT_GRAPHICS_MODE);
}

inline void vga_set_pixel_at(unsigned short x, unsigned short y, unsigned char color_index) {
    syscall(middlespace::SysCallNumbers::VGA_SET_PIXEL_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)color_index);
}

inline void exit(u64 code) {
    syscall(middlespace::SysCallNumbers::EXIT, (syscall_arg)code);
}

inline s64 elf_run(const char path[], const char* nullterm_argv[]) {
    return syscall(middlespace::SysCallNumbers::ELF_RUN, (syscall_arg)path, (syscall_arg)nullterm_argv);
}

inline s64 task_lightweight_run(unsigned long long entry_point, unsigned long long arg = 0, const char name[] = "lightweight") {
    return syscall(middlespace::SysCallNumbers::TASK_LIGHTWEIGHT_RUN, (syscall_arg)entry_point, (syscall_arg)arg, (syscall_arg)name);
}

inline s64 task_wait(unsigned int task_id) {
    return syscall(middlespace::SysCallNumbers::TASK_WAIT, (syscall_arg)task_id);
}
} // namespace syscalls



#endif /* ELFS_SYSCALLS_H_ */
