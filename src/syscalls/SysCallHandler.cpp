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
#include "posix/posix.h"

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
        if (!files[i]) {    // found empty file descriptor
            string absolute_path = make_absolute_path(name);
            VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);

            if (entry && entry->open()) {
                files[i] = entry;
                return i;
            } else
                return -1; // cant get or open such file
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

    files[fd]->close();
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

off_t SysCallHandler::sys_lseek(int fd, off_t offset, int whence) {
    auto& files = current().files;

    if (fd >= files.size())
        return -1;

    if (!files[fd])
        return -1;

    u32 new_position;
    switch (whence) {
    case SEEK_SET:
        new_position = offset;
        break;

    case SEEK_CUR:
        new_position = files[fd]->get_position() + offset;
        break;

    case SEEK_END:
        new_position = files[fd]->get_size() + offset;
        break;

    default:
        return -1;
    }

    if (files[fd]->seek(new_position))
        return new_position;
    else
        return -1;
}

/**
 * @brief   Get file status
 * @return  On success, zero is returned. On error, -1 is returned
 * @see     http://man7.org/linux/man-pages/man2/stat.2.html
 */
s32 SysCallHandler::sys_stat(const char* name, struct stat* buff) {
    string absolute_path = make_absolute_path(name);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry)
        return -1;

    memset(buff, 0, sizeof(struct stat));
    buff->st_size = entry->get_size();
    buff->st_mode = entry->is_directory() ? S_IFDIR : S_IFREG;

    return 0;
}

/**
 * @brief   Move/rename filesystem entry
 * @return  On success, zero is returned. On error, -1 is returned
 * @see     http://man7.org/linux/man-pages/man2/rename.2.html
 */
s32 SysCallHandler::sys_rename(const char old_path[], const char new_path[]) {
    string absolute_old_path = make_absolute_path(old_path);
    string absolute_new_path = make_absolute_path(new_path);
    if (VfsManager::instance().move_entry(absolute_old_path, absolute_new_path))
        return 0;
    else
        return -1;
}

/**
 * @brief   Create directory
 * @return  On success, zero is returned. On error, -1 is returned
 * @see     http://man7.org/linux/man-pages/man2/mkdir.2.html
 */
s32 SysCallHandler::sys_mkdir(const char path[], int mode) {
    string absolute_path = make_absolute_path(path);
    if (VfsManager::instance().create_entry(absolute_path, true))
        return 0;
    else
        return -1;
}

/**
 * @brief   Remove directory which must be empty
 * @return  On success, zero is returned. On error, -1 is returned
 * @see     http://man7.org/linux/man-pages/man2/rmdir.2.html
 */
s32 SysCallHandler::sys_rmdir(const char path[]) {
    string absolute_path = make_absolute_path(path);
    if (VfsManager::instance().delete_entry(absolute_path))
        return 0;
    else
        return -1;
}

/**
 * @brief   Create file and return its descriptor
 * @param   name NOT USED NOW
 * @param   mode NOT USED NOW
 * @return  Descriptor number on success, -1 otherwise
 * @see     http://man7.org/linux/man-pages/man2/open.2.html
 */
s32 SysCallHandler::sys_creat(const char* name, int mode) {
    // create file
    string absolute_path = make_absolute_path(name);
    auto entry = VfsManager::instance().create_entry(absolute_path, false);
    if (!entry || !entry->open())
        return -1;

    // alloc the created file in descriptor table
    auto& files = current().files;
    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {    // found empty file descriptor
            files[i] = entry;
            return i;
        }

    return -1;
}

/**
 * @brief   Get current working directory and store into "buff"
 * @param   size Size of the "buff"
 * @return  Pointer to "buff" on success, nullptr on error
 */
char* SysCallHandler::sys_get_cwd(char* buff, size_t size) {
    if (!buff)
        return nullptr;

    const string& cwd = current().cwd;
    if (size < cwd.length() + 1)
        return nullptr;

    memcpy(buff, cwd.c_str(), cwd.length() + 1); // +1 copy with null
    return buff;
}

/**
 * @brief   Change current working directory
 * @return  0 on success, -1 on error
 */
s32 SysCallHandler::sys_chdir(const char path[]) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry)
        return -1;

    if (!entry->is_directory())
        return -1;

    current().cwd = absolute_path;
    return 0;
}

void SysCallHandler::sys_exit(s32 status) {
    multitasking::Task::exit(status);
}

void SysCallHandler::sys_exit_group(s32 status) {
    multitasking::Task::exit(status);
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

void SysCallHandler::vga_flush_buffer(const u16* buff) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->flush_buffer((VgaCharacter*)buff);
    }
}

void SysCallHandler::vga_get_width_height(u16* width, u16* height) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        *width = drv->screen_width();
        *height = drv->screen_height();
    }
    else {
        *width = 0;
        *height = 0;
    }
}

/**
 * @brief   Run ELF64 format program pointed by "absolute_filename"
 * @param   name ELF64 filename to run
 * @param   nullterm_argv List of cstrings, last one == null
 */
s64 SysCallHandler::elf_run(const char name[], const char* nullterm_argv[]) {
    string absolute_path = make_absolute_path(name);
    VfsEntryPtr e = VfsManager::instance().get_entry(absolute_path);
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

/**
 * @brief   Return absolute path made from "path" and current task working directory
 */
UnixPath SysCallHandler::make_absolute_path(const kstd::string& path) const {
    const string& cwd = current().cwd;

    // check if relative_filename specified at all
    if (path.empty())
        return cwd;

    if (path == ".")
        return cwd;

    // check if relative_filename starts with a slash, which means it is actually an absolute filename
    if (path[0] == '/')
        return path;

    // make absolute filename from current working directory + relative filename
    return cwd + "/" + path;
}
} /* namespace filesystem */
