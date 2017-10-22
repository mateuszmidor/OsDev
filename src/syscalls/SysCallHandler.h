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
#include "middlespace/FsEntry.h"

namespace syscalls {

class SysCallHandler {
public:
    SysCallHandler();

    s32 sys_open(const char name[], int flags, int mode);
    s32 sys_close(u32 fd);
    s64 sys_read(u32 fd, void *buf, u64 count);
    s64 sys_write(u32 fd, const void *buf, u64 count);
    s32 sys_enumerate(u32 fd, middlespace::FsEntry* entries, u32 max_entries);

    void sys_exit(s32 status);
    void sys_exit_group(s32 status);

    void vga_cursor_setvisible(bool visible);
    void vga_cursor_setpos(u8 x, u8 y);
    void vga_setat(u8 x, u8 y, u16 c);

private:
    multitasking::Task& current() const;
};

} /* namespace filesystem */

#endif /* SRC_SYSCALLS_SYSCALLHANDLER_H_ */
