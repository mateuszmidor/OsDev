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
#include "FsEntry.h"
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

    s32 sys_get_cwd(char* buff, size_t size);
    s32 sys_chdir(const char path[]);
    void sys_exit(s32 status);
    void sys_exit_group(s32 status);

    s32 enumerate(u32 fd, middlespace::FsEntry* entries, u32 max_entries);
    void vga_cursor_setvisible(bool visible);
    void vga_cursor_setpos(u8 x, u8 y);
    void vga_setat(u8 x, u8 y, u16 c);
    void vga_flush_buffer(const u16* buff);
    void vga_get_width_height(u16* width, u16* height);
    s64 elf_run(const char path[], const char* nullterm_argv[]);

private:
    multitasking::Task& current() const;
    filesystem::UnixPath make_absolute_path(const kstd::string& path) const;
};

} /* namespace filesystem */

#endif /* SRC_SYSCALLS_SYSCALLHANDLER_H_ */
