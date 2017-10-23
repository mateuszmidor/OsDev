/**
 *   @file: SysCallHandler.cpp
 *
 *   @date: Oct 22, 2017
 * @author: Mateusz Midor
 */

#include "SysCallHandler.h"
#include "TaskManager.h"
#include "VfsManager.h"
#include "DriverManager.h"
#include "VgaDriver.h"
#include "ElfRunner.h"
#include "syscall_errors.h"

using namespace kstd;
using namespace drivers;
using namespace filesystem;
namespace syscalls {

SysCallHandler::SysCallHandler() { }

/*************************************************************************************
 * SYSCALLS RELATED TO POSIX
*************************************************************************************/

/**
 * @brief   Get file with given absolute "name", associate file descriptor to it and return the descriptor
 * @return  Descriptor number on success, -1 otherwise
 */
s32 SysCallHandler::sys_open(const char name[], int flags, int mode) {
    auto& files = current().files;

    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {
            if (VfsEntryPtr entry = VfsManager::instance().get_entry(name)) {
                files[i] = entry;
                return i;
            } else
                return -1; // cant get such file
        }
    return -1; // open file limit reached
}

/**
 * @brief   Close file associated with "fd" file descriptor
 * @return  0 if valid "fd" provided, -1 otherwise
 */
s32 SysCallHandler::sys_close(u32 fd) {
    auto& files = current().files;

    if (fd >= files.size())
        return -1;

    if (!files[fd])
        return -1;

    files[fd].reset();
    return 0;
}

s64 SysCallHandler::sys_read(u32 fd, void *buf, u64 count) {
    auto& files = current().files;

    if (fd >= files.size())
        return 0;

    if (!files[fd])
        return 0;

    return files[fd]->read(buf, count);
}

s64 SysCallHandler::sys_write(u32 fd, const void *buf, u64 count) {
    auto& files = current().files;

    if (fd >= files.size())
        return 0;

    if (!files[fd])
        return 0;

    return files[fd]->write(buf, count);
}

void SysCallHandler::sys_exit(s32 status) {
    multitasking::Task::exit();
}

void SysCallHandler::sys_exit_group(s32 status) {
    multitasking::Task::exit();
}

/*************************************************************************************
 * SYSCALLS THAT HAVE NOTHING TO DO WITH POSIX
*************************************************************************************/
/**
 * @brief   Enumerate contents of directory pointed by "fd"
 * @param   fd Directory filedescriptor
 * @param   entries Output buffer for directory entries
 * @param   max_entries "entries" buffer capacity, max num items
 */
s32 SysCallHandler::sys_enumerate(u32 fd, middlespace::FsEntry* entries, u32 max_entries) {
    auto& files = current().files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

    if (!files[fd]->is_directory())
        return -ENOTDIR;

    u32 entry_no = 0;

    OnVfsEntryFound on_entry = [&](VfsEntryPtr e) -> bool {
        if (entry_no > max_entries)
            return false;   // stop iterating

        entries[entry_no].is_directory = e->is_directory();
        entries[entry_no].size = e->get_size();

        // TODO: possible buff overflow, implement and use strncpy
        memcpy(entries[entry_no].name, e->get_name().c_str(), e->get_name().length() + 1);
        entry_no++;

        return true;
    };

    if (files[fd]->enumerate_entries(on_entry) == VfsEnumerateResult::ENUMERATION_FINISHED)
        return entry_no;
    else
        return -EINVAL; // buffer too small
}

void SysCallHandler::vga_cursor_setvisible(bool visible) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->set_cursor_visible(visible);
    }
}

void SysCallHandler::vga_cursor_setpos(u8 x, u8 y) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->set_cursor_pos(x, y);
    }
}

void SysCallHandler::vga_setat(u8 x, u8 y, u16 c) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        VgaCharacter* vc = (VgaCharacter*)&c;
        drv->at(x, y) = *vc;
    }
}

/**
 * @brief   Run ELF64 format program pointed by "absolute_filename"
 * @param   absolute_filename Absolute filename starting at root "/"
 * @param   nullterm_argv List of cstrings, last one == null
 */
s64 SysCallHandler::elf_run(const char absolute_filename[], const char* nullterm_argv[]) {
    VfsEntryPtr e = VfsManager::instance().get_entry(absolute_filename);
    if (!e)
        return -ENOENT; // no such file

    if (e->is_directory())
        return -EISDIR; // is directory

    u32 size = e->get_size();
    u8* elf_data = new u8[size];
    if (!elf_data)
        return -ENOMEM;
    e->read(elf_data, size);

    vector<string>* args = new vector<string>;
    while (*nullterm_argv) {
        args->push_back(*nullterm_argv);
        nullterm_argv++;
    }

    utils::ElfRunner runner;
    return runner.run(elf_data, args);
}

/**
 * @brief   Return current task, that called the syscall
 */
multitasking::Task& SysCallHandler::current() const {
    multitasking::TaskManager& mngr = multitasking::TaskManager::instance();
    return mngr.get_current_task();
}

} /* namespace filesystem */
