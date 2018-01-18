/**
 *   @file: syscalls.cpp
 *
 *   @date: Dec 6, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"

namespace cstd {
namespace ustd {
namespace syscalls {


/**
 * @brief   Sleep current task for at least given amount of nanoseconds
 *          It must be implemented as int80h until kernel is preemptible and syscall can be suspended
 * @note    when nsec == 0, it only reschedules CPU to another task
 */
int nsleep(unsigned long long nsec) {
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
int msleep(unsigned long long msec) {
    return nsleep(msec * 1000 * 1000);
}

/**
 * @brief   Reschedule CPU to another task
 */
void yield() {
    nsleep(0);
}

/**
 * @note    The below syscalls are received in SysCallManager and passed for handling to SysCallHandler
 */
syscall_res syscall(middlespace::SysCallNumbers syscall,
        syscall_arg arg1,
        syscall_arg arg2,
        syscall_arg arg3,
        syscall_arg arg4,
        syscall_arg arg5) {

    syscall_res result;

    do {
        // 1. make system call
        asm volatile(
                "mov %%rbx, %%r10       ;"
                "mov %%rcx, %%r8        ;"
                "syscall                ;"
                : "=a"(result)
                : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "b"(arg4), "c"(arg5)
                : "r11" // r11 is internally used by syscall for RFLAGS, and RCX for RIP
        );

        // 2. reschedule if syscall would block
        if (result == -EWOULDBLOCK)
            yield();

    } while (result == -EWOULDBLOCK);

    return result;
}

ssize_t read(int fd, void* buf, size_t count) {
    return syscall(middlespace::SysCallNumbers::FILE_READ, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return syscall(middlespace::SysCallNumbers::FILE_WRITE, (syscall_arg)fd, (syscall_arg)buf, (syscall_arg)count);
}

int open(const char* absolute_path, int flags) {
    return syscall(middlespace::SysCallNumbers::FILE_OPEN, (syscall_arg)absolute_path, (syscall_arg)flags);
}

int close(int fd) {
    return syscall(middlespace::SysCallNumbers::FILE_CLOSE, (syscall_arg)fd);
}

/**
 * @brief   Change current position in file
 * @param   whence: SEEK_SET(new pos = 0 + offset), SEEK_CUR(new pos = current pos + offset), SEEK_END(new pos = file end + offset)
 * @return  New current position (in bytes), or -1 on error
 */
off_t lseek(int fd, off_t offset, int whence) {
    return syscall(middlespace::SysCallNumbers::FILE_SEEK,(syscall_arg)fd, (syscall_arg)offset, (syscall_arg)whence);
}

/**
 * @brief   Get virtual file system entry info
 * @return  0 on success, -1 on no such file
 */
int stat(const char path[], struct stat* statbuf) {
    return syscall(middlespace::SysCallNumbers::FILE_STAT, (syscall_arg)path, (syscall_arg)statbuf);
}

/**
 * @brief   Truncate file to specified size
 * @return  0 on success, negative error code on error
 */
int truncate(const char path[], off_t length) {
    return syscall(middlespace::SysCallNumbers::FILE_TRUNCATE, (syscall_arg)path, (syscall_arg)length);
}
/**
 * @brief   Rename file/directory
 * @return  0 on success, negative error code on error
 */
int rename(const char oldname[], const char newname[]) {
    return syscall(middlespace::SysCallNumbers::FILE_RENAME, (syscall_arg)oldname, (syscall_arg)newname);
}

/**
 * @brief   Create directory
 * @return  0 on success, negative error code on error
 * @note    "mode" is not used now
 */
int mkdir(const char path[], int mode) {
    return syscall(middlespace::SysCallNumbers::FILE_MKDIR, (syscall_arg)path, (syscall_arg)mode);
}

/**
 * @brief   Remove directory
 * @return  0 on success, negative error code on error
 * @note    Directory must  be empty. Removes only directories. For file use unlink
 */
int rmdir(const char path[]) {
    return syscall(middlespace::SysCallNumbers::FILE_RMDIR, (syscall_arg)path);
}

int creat(const char path[], int mode) {
    return syscall(middlespace::SysCallNumbers::FILE_CREAT, (syscall_arg)path, (syscall_arg)mode);
}

/**
 * @brief   Remove file
 * @return  0 on success, negative error code on error
 * @note    Removes only files, for directory use rmdir
 */
int unlink(const char path[]) {
    return syscall(middlespace::SysCallNumbers::FILE_UNLINK, (syscall_arg)path);
}

/**
 * @brief   Move program break effectively changing amount of dynamic memory available to the task
 * @param   new_brk New dynamic memory high limit
 * @return  New program break on success, current program break on failure (no memory, new_brk == 0)
 */
size_t brk(size_t new_brk) {
    return syscall(middlespace::SysCallNumbers::BRK, (syscall_arg)new_brk);
}
/**
 * @brief   Get current working directory
 * @param   buff Buffer for storing the cwd
 * @param   size Length of the "buff", should be at least 256
 * @return  0 on success, negative error code on error
 */
int getcwd(char* buff, size_t size) {
    return syscall(middlespace::SysCallNumbers::GET_CWD, (syscall_arg)buff, (syscall_arg)size);
}

/**
 * @brief   Change current working directory
 * @return  0 on success, -1 on error
 */
int chdir(const char path[]) {
    return syscall(middlespace::SysCallNumbers::CHDIR, (syscall_arg)path);
}

/**
 * @brief   Get time of specified "clk_id"
 * @return  0 on success, -EINVAL on invalid/unsupported "clk_id", -EFAULT on invalid "tp"
 * @note    only CLOCK_MONOTONIC is supported now as "clk_id"
 */
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    return syscall(middlespace::SysCallNumbers::CLOCK_GETTIME, (syscall_arg)clk_id, (syscall_arg)tp);
}

int enumerate(unsigned int fd, middlespace::VfsEntry* entries, unsigned int max_enties) {
    return syscall(middlespace::SysCallNumbers::FILE_ENUMERATE, (syscall_arg)fd, (syscall_arg)entries, (syscall_arg)max_enties);
}

void vga_cursor_setvisible(bool visible) {
    syscall(middlespace::SysCallNumbers::VGA_CURSOR_SETVISIBLE, (syscall_arg)visible);
}

void vga_cursor_setpos(unsigned int x, unsigned int y) {
    syscall(middlespace::SysCallNumbers::VGA_CURSOR_SETPOS, (syscall_arg)x, (syscall_arg)y);
}

unsigned short vga_get_char_at(unsigned int x, unsigned int y) {
    return syscall(middlespace::SysCallNumbers::VGA_GET_CHAR_AT, (syscall_arg)x, (syscall_arg)y);
}

void vga_set_char_at(unsigned int x, unsigned int y, unsigned short vga_character) {
    syscall(middlespace::SysCallNumbers::VGA_SET_CHAR_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)vga_character);
}

void vga_flush_char_buffer(const unsigned short* text_buffer) {
    syscall(middlespace::SysCallNumbers::VGA_FLUSH_CHAR_BUFFER, (syscall_arg)text_buffer);
}

void vga_flush_video_buffer(const unsigned char* video_buffer) {
    syscall(middlespace::SysCallNumbers::VGA_FLUSH_VIDEO_BUFFER, (syscall_arg)video_buffer);
}

void vga_get_width_height(unsigned short* width, unsigned short* height) {
    syscall(middlespace::SysCallNumbers::VGA_GET_WIDTH_HEIGHT, (syscall_arg)width, (syscall_arg)height);
}

void vga_enter_graphics_mode() {
    syscall(middlespace::SysCallNumbers::VGA_ENTER_GRAPHICS_MODE);
}

void vga_exit_graphics_mode() {
    syscall(middlespace::SysCallNumbers::VGA_EXIT_GRAPHICS_MODE);
}

void vga_set_pixel_at(unsigned short x, unsigned short y, unsigned char color_index) {
    syscall(middlespace::SysCallNumbers::VGA_SET_PIXEL_AT, (syscall_arg)x, (syscall_arg)y, (syscall_arg)color_index);
}

void exit(u64 code) {
    syscall(middlespace::SysCallNumbers::EXIT, (syscall_arg)code);
}

void exit_group(u64 code) {
    syscall(middlespace::SysCallNumbers::EXIT_GROUP, (syscall_arg)code);
}

s64 elf_run(const char path[], const char* nullterm_argv[]) {
    return syscall(middlespace::SysCallNumbers::ELF_RUN, (syscall_arg)path, (syscall_arg)nullterm_argv);
}

s64 task_lightweight_run(unsigned long long entry_point, unsigned long long arg, const char name[]) {
    return syscall(middlespace::SysCallNumbers::TASK_LIGHTWEIGHT_RUN, (syscall_arg)entry_point, (syscall_arg)arg, (syscall_arg)name);
}

s64 task_wait(unsigned int task_id) {
    return syscall(middlespace::SysCallNumbers::TASK_WAIT, (syscall_arg)task_id);
}

}
}
}
