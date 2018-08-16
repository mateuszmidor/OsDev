/**
 *   @file: syscalls.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_SYSCALLS_H_
#define ELFS_SYSCALLS_H_

#include "types.h"
#include "SysCallNumbers.h"
#include "Vfs.h"
#include "posix.h"

namespace cstd {
namespace ustd {
namespace syscalls {

using syscall_res = signed long long;
using syscall_arg = unsigned long long;


/**
 * @brief   Sleep current task for at least given amount of nanoseconds
 *          It must be implemented as int80h until kernel is preemptible and syscall can be suspended
 * @note    when nsec == 0, it only reschedules CPU to another task
 */
int nsleep(unsigned long long nsec);

/**
 * @brief   Sleep current task for at least given amount of milliseconds
 *          It must be implemented as int80h until kernel is preemptible and syscall can be suspedned
 * @note    when msec == 0, it only reschedules CPU to another task
 */
int msleep(unsigned long long msec);

/**
 * @brief   Reschedule CPU to another task
 */
void yield();

/**
 * @note    The below syscalls are received in SysCallManager and passed for handling to SysCallHandler
 */
syscall_res syscall(middlespace::SysCallNumbers syscall,
        syscall_arg arg1 = 0,
        syscall_arg arg2 = 0,
        syscall_arg arg3 = 0,
        syscall_arg arg4 = 0,
        syscall_arg arg5 = 0);

ssize_t read(int fd, void* buf, size_t count);

ssize_t write(int fd, const void* buf, size_t count);

int open(const char* absolute_path, int flags = 0);

int close(int fd);

/**
 * @brief   Change current position in file
 * @param   whence: SEEK_SET(new pos = 0 + offset), SEEK_CUR(new pos = current pos + offset), SEEK_END(new pos = file end + offset)
 * @return  New current position (in bytes), or -1 on error
 */
off_t lseek(int fd, off_t offset, int whence);

/**
 * @brief   Get virtual file system entry info
 * @return  0 on success, -1 on no such file
 */
int stat(const char path[], struct stat* statbuf);

/**
 * @brief   Truncate file to specified size
 * @return  0 on success, negative error code on error
 */
int truncate(const char path[], off_t length);

/**
 * @brief   Rename file/directory
 * @return  0 on success, negative error code on error
 */
int rename(const char oldname[], const char newname[]);

/**
 * @brief   Create directory
 * @return  0 on success, negative error code on error
 * @note    "mode" is not used now
 */
int mkdir(const char path[], int mode = 0);

/**
 * @brief   Remove directory
 * @return  0 on success, negative error code on error
 * @note    Directory must  be empty. Removes only directories. For file use unlink
 */
int rmdir(const char path[]);

int creat(const char path[], int mode = 0);

/**
 * @brief   Remove file
 * @return  0 on success, negative error code on error
 * @note    Removes only files, for directory use rmdir
 */
int unlink(const char path[]);

/**
 *
 * @param   mode  Must be S_IFIFO
 * @param   dev   Ignored
 * @return  0 on success, negative error code on error
 */
int mknod(const char path[], int mode = S_IFIFO, int dev = 0);
/**
 * @brief   Move program break effectively changing amount of dynamic memory available to the task
 * @param   new_brk New dynamic memory high limit
 * @return  New program break on success, current program break on failure (no memory, new_brk == 0)
 */
size_t brk(size_t new_brk);

/**
 * @brief   Get current working directory
 * @param   buff Buffer for storing the cwd
 * @param   size Length of the "buff", should be at least 256
 * @return  0 on success, negative error code on error
 */
int getcwd(char* buff, size_t size);

/**
 * @brief   Change current working directory
 * @return  0 on success, -1 on error
 */
int chdir(const char path[]);

int clock_gettime(clockid_t clk_id, struct timespec *tp);

int enumerate(unsigned int fd, middlespace::VfsEntry* entries, unsigned int max_enties);

void vga_cursor_setvisible(bool visible);

void vga_cursor_setpos(unsigned int x, unsigned int y);

unsigned short vga_get_char_at(unsigned int x, unsigned int y);

void vga_set_char_at(unsigned int x, unsigned int y, unsigned short vga_character);

void vga_flush_char_buffer(const unsigned short* text_buffer);

void vga_flush_video_buffer(const unsigned char* video_buffer);

void vga_get_width_height(unsigned short* width, unsigned short* height);

void vga_enter_graphics_mode();

void vga_exit_graphics_mode();

void vga_set_pixel_at(unsigned short x, unsigned short y, unsigned char color_index);

void exit(u64 code);

void exit_group(u64 code);

s64 elf_run(const char path[], const char* nullterm_argv[]);

s64 task_lightweight_run(unsigned long long entry_point, unsigned long long arg = 0, const char name[] = "lightweight");

s64 task_wait(unsigned int task_id);
} // namespace syscalls
} // namespace ustd
} // namespace cstd


#endif /* ELFS_SYSCALLS_H_ */
