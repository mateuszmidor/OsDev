/**
 *   @file: SysCallHandler.h
 *
 *   @date: Oct 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_SYSCALLS_SYSCALLHANDLER_H_
#define SRC_SYSCALLS_SYSCALLHANDLER_H_

#include "types.h"
#include "TaskManager.h"
#include "UnixPath.h"
#include "Vfs.h"
#include "posix/posix.h"

namespace syscalls {

class SysCallHandler {
public:
    SysCallHandler();

    s32 sys_open(const char path[], int flags, int mode);
    s32 sys_close(u32 fd);
    s64 sys_read(u32 fd, void* buf, u64 count);
    s64 sys_write(u32 fd, const void* buf, u64 count);
    off_t sys_lseek(int fd, off_t offset, int whence);
    s32 sys_stat(const char path[], struct stat* buff);
    s32 sys_truncate(const char path[], off_t length);
    s32 sys_rename(const char old_path[], const char new_path[]);
    s32 sys_mkdir(const char path[], int mode);
    s32 sys_rmdir(const char path[]);
    s32 sys_creat(const char path[], int mode);
    s32 sys_unlink(const char path[]);
    s32 sys_mknod(const char path[], int mode, int dev);
    u64 sys_brk(u64 new_brk);
    s32 sys_get_cwd(char* buff, size_t size);
    s32 sys_chdir(const char path[]);
    s32 sys_clock_gettime(clockid_t clk_id, struct timespec* tp);
    void sys_exit(s32 status);
    void sys_exit_group(s32 status);

    s32 enumerate(u32 fd, middlespace::VfsEntry* entries, u32 max_entries);
    void vga_cursor_setvisible(bool visible);
    void vga_cursor_setpos(u8 x, u8 y);
    u16 vga_get_char_at(u8 x, u8 y);
    void vga_set_char_at(u8 x, u8 y, u16 c);
    void vga_flush_char_buffer(const u16* buff);
    void vga_flush_video_buffer(const u8* buff);
    void vga_get_width_height(u16* width, u16* height);
    void vga_enter_graphics_mode();
    void vga_exit_graphics_mode();
    void vga_set_pixel_at(u16 x, u16 y, u8 c);
    s64 elf_run(const char path[], const char* nullterm_argv[]);
    s64 task_wait(u32 task_id);
    s64 task_lightweight_run(u64 entry_point, u64 arg, const char name[]);

private:
    multitasking::Task& current() const;
    filesystem::UnixPath make_absolute_path(const cstd::string& path) const;
};

} /* namespace filesystem */

#endif /* SRC_SYSCALLS_SYSCALLHANDLER_H_ */
