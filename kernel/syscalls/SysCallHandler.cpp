/**
 *   @file: SysCallHandler.cpp
 *
 *   @date: Oct 22, 2017
 * @author: Mateusz Midor
 */

#include "SysCallHandler.h"
#include "TaskManager.h"
#include "TaskFactory.h"
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
 * @brief   Open filesystem entry with given "path", assign file descriptor to it and return the descriptor
 * @return  Descriptor number on success
            -EMFILE if per-process open file limit is reached
            -ENOENT if the filesystem entry does not exist
            -EACCES if the filesystem entry can't be opened
  @note     As for now, flags and mode is unused
  @see      http://man7.org/linux/man-pages/man2/open.2.html
 */
s32 SysCallHandler::sys_open(const char path[], int flags, int mode) {
    auto& files = current().task_group_data->files;

    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {    // found empty file descriptor
            string absolute_path = make_absolute_path(path);
            VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);

            if (!entry)
                return -ENOENT; // no such entry

            if (!entry->open()) // cant open
                return -EACCES;

            files[i] = entry;
            return i;
        }
    return -EMFILE; // open file limit reached
}

/**
 * @brief   Close filesystem entry associated with "fd" file descriptor and release the descriptor
 * @return  0 on success
            -EBADF if "fd" is not valid descriptor
 * @see     http://man7.org/linux/man-pages/man2/close.2.html
 */
s32 SysCallHandler::sys_close(u32 fd) {
    auto& files = current().task_group_data->files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

    files[fd]->close();
    files[fd].reset();
    return 0;
}

/**
 * @brief   Read up to "count" bytes
 * @return  Number of actually read bytes on success
 *          -EBADF if "fd" is not valid descriptor
 * @see     http://man7.org/linux/man-pages/man2/read.2.html
 */
s64 SysCallHandler::sys_read(u32 fd, void *buf, u64 count) {
    auto& files = current().task_group_data->files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

    return files[fd]->read(buf, count);
}

/**
 * @brief   Write up to "count" bytes
 * @return  Number of actually written bytes on success
 *          -EBADF if "fd" is not valid descriptor
 * @see     http://man7.org/linux/man-pages/man2/write.2.html
 */
s64 SysCallHandler::sys_write(u32 fd, const void *buf, u64 count) {
    auto& files = current().task_group_data->files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

    return files[fd]->write(buf, count);
}

/**
 * @brief   Change current read/write position
 * @return  New current position from the file beginning on success
 *          -EBADF if "fd" is not valid descriptor
 *          -EINVAL if "whence" is invalid
 * @see     http://man7.org/linux/man-pages/man2/lseek.2.html
 */
off_t SysCallHandler::sys_lseek(int fd, off_t offset, int whence) {
    auto& files = current().task_group_data->files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

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
        return -EINVAL;
    }

    if (files[fd]->seek(new_position))
        return new_position;
    else
        return -EINVAL;
}

/**
 * @brief   Get file status
 * @return  0 on success
 *          -ENOENT if the filesystem entry does not exist
 * @see     http://man7.org/linux/man-pages/man2/stat.2.html
 */
s32 SysCallHandler::sys_stat(const char path[], struct stat* buff) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry)
        return -ENOENT;

    memset(buff, 0, sizeof(struct stat));
    buff->st_size = entry->get_size();
    buff->st_mode = entry->is_directory() ? S_IFDIR : S_IFREG;

    return 0;
}

/**
 * @brief   Truncate file to given "lenght"
 * @return  0 on success
 *          -ENOENT if the filesystem entry does not exist
 *          -EISDIR if the filesystem entry is directory
 *          -EINVAL if "length" is invalid
 * @see     http://man7.org/linux/man-pages/man2/truncate.2.html
 */
s32 SysCallHandler::sys_truncate(const char path[], off_t length) {
    if (length < 0)
        return -EINVAL;

    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry)
        return -ENOENT;

    if (entry->is_directory())
        return -EISDIR;

    if (entry->truncate(length))
        return 0;
    else
        return -EINVAL;
}

/**
 * @brief   Move/rename filesystem entry
 * @return  0 on success
 *          -EACCES if some trouble happened
 * @see     http://man7.org/linux/man-pages/man2/rename.2.html
 */
s32 SysCallHandler::sys_rename(const char old_path[], const char new_path[]) {
    string absolute_old_path = make_absolute_path(old_path);
    string absolute_new_path = make_absolute_path(new_path);
    if (VfsManager::instance().move_entry(absolute_old_path, absolute_new_path))
        return 0;
    else
        return -EACCES;
}

/**
 * @brief   Create directory
 * @return  0 on success
 *          -EACCES if some trouble happened
 * @see     http://man7.org/linux/man-pages/man2/mkdir.2.html
 */
s32 SysCallHandler::sys_mkdir(const char path[], int mode) {
    string absolute_path = make_absolute_path(path);
    if (VfsManager::instance().create_entry(absolute_path, true))
        return 0;
    else
        return -EACCES;
}

/**
 * @brief   Remove directory which must be empty
 * @return  0 on success
 *          -ENOTDIR if filesystem entry is not directory
 *          -EACCES if some trouble happened
 * @see     http://man7.org/linux/man-pages/man2/rmdir.2.html
 */
s32 SysCallHandler::sys_rmdir(const char path[]) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry->is_directory())
        return -ENOTDIR;

    if (VfsManager::instance().delete_entry(absolute_path))
        return 0;
    else
        return -EACCES;
}

/**
 * @brief   Create file and return its descriptor
 * @param   mode NOT USED NOW
 * @return  Descriptor number on success
 *          -EMFILE if per-process open file limit is reached
            -EACCES if some trouble happened
 * @see     http://man7.org/linux/man-pages/man2/open.2.html
 */
s32 SysCallHandler::sys_creat(const char path[], int mode) {
    // create file
    string absolute_path = make_absolute_path(path);
    auto entry = VfsManager::instance().create_entry(absolute_path, false);
    if (!entry)
        return -EACCES;

    if (!entry->open())
        return -EACCES;

    // alloc the created file in descriptor table
    auto& files = current().task_group_data->files;
    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {    // found empty file descriptor
            files[i] = entry;
            return i;
        }

    return -EMFILE;
}

/**
 * @brief   Remove file from filesystem
 * @return  0 on success
 *          -ENOENT if filesystem entry doesnt exist
 *          -EISDIR if filesystem entry is directory
 *          -EACCES if some trouble happened
 * @see     https://linux.die.net/man/2/unlink
 */
s32 SysCallHandler::sys_unlink(const char path[]) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);

    if (!entry)
        return -ENOENT; // no such entry

    if (entry->is_directory())
        return -EISDIR; // is directory

    if (VfsManager::instance().delete_entry(absolute_path))
        return 0;
    else
        return -EACCES;
}

/**
 * @brief   Get current working directory and store into "buff"
 * @param   size Size of the "buff"
 * @return  0 on success
 *          -EINVAL if "buff" is null
 *          -ERANGE if "size" < cwd.length
 * @see     http://man7.org/linux/man-pages/man3/getcwd.3.html
 */
s32 SysCallHandler::sys_get_cwd(char* buff, size_t size) {
    if (!buff)
        return -EINVAL;

    const string& cwd = current().task_group_data->cwd;
    if (size < cwd.length() + 1)
        return -ERANGE;

    memcpy(buff, cwd.c_str(), cwd.length() + 1); // +1 copy with null
    return 0;
}

/**
 * @brief   Change current working directory
 * @return  0 on success
            -ENOENT if filesystem entry doesnt exist
            -ENOTDIR if filesystem entry is not directory
 * @see     http://man7.org/linux/man-pages/man2/chdir.2.html
 */
s32 SysCallHandler::sys_chdir(const char path[]) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr entry = VfsManager::instance().get_entry(absolute_path);
    if (!entry)
        return -ENOENT ;

    if (!entry->is_directory())
        return -ENOTDIR;

    current().task_group_data->cwd = absolute_path;
    return 0;
}

/**
 * @brief   Exit current task
 * @return  This function does not return; TaskManager schedules another task instead
 * @see     http://man7.org/linux/man-pages/man3/exit.3.html
 */
void SysCallHandler::sys_exit(s32 status) {
    multitasking::Task::exit(status);
}

/**
 * @brief   Exit all threads in current process. Right now there is no multiple threads per process, so exit current task
 * @return  This function does not return; TaskManager schedules another task instead
 * @see     http://man7.org/linux/man-pages/man2/exit_group.2.html
 */
void SysCallHandler::sys_exit_group(s32 status) {
    multitasking::Task::exit_group(status);
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
s32 SysCallHandler::enumerate(u32 fd, middlespace::FsEntry* entries, u32 max_entries) {
    auto& files = current().task_group_data->files;

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

void SysCallHandler::vga_set_char_at(u8 x, u8 y, u16 c) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        VgaCharacter* vc = (VgaCharacter*)&c;
        drv->at(x, y) = *vc;
    }
}

void SysCallHandler::vga_flush_char_buffer(const u16* buff) {
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

void SysCallHandler::vga_enter_graphics_mode() {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->set_graphics_mode_320_200_256();
    }
}

void SysCallHandler::vga_exit_graphics_mode() {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->set_text_mode_90_30();
        drv->clear_screen(EgaColor::Black);
    }
}

void SysCallHandler::vga_set_pixel_at(u16 x, u16 y, u8 c) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->put_pixel(x, y, c);
    }
}

/**
 * @brief   Run ELF64 format program pointed by "path"
 * @param   nullterm_argv List of cstrings, last one == null
 */
s64 SysCallHandler::elf_run(const char path[], const char* nullterm_argv[]) {
    string absolute_path = make_absolute_path(path);
    VfsEntryPtr e = VfsManager::instance().get_entry(absolute_path);
    if (!e)
        return -ENOENT; // no such file

    if (e->is_directory())
        return -EISDIR; // is directory

    // TODO: reading elf file should be done from kernel task not from syscall, this is experimental version
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
 * @brief   Suspend current task until "task_id" is finished
 * @return  -EWOULDBLOCK on success
            -EINVAL on invalid "task_id"
 */
s64 SysCallHandler::task_wait(u32 task_id) {
    multitasking::TaskManager& mngr = multitasking::TaskManager::instance();
    if (mngr.wait(task_id))
        return -EWOULDBLOCK;
    else
        return -EINVAL;
}

/**
 *
 * @brief   Run task process in current task address space
 * @return  Task ID on success
 *          -EINVAL if "entry_point" or "name" is null
 *          -EPERM if task could not be run
 */
s64 SysCallHandler::task_lightweight_run(u64 entry_point, u64 arg, const char name[]) {
    if (!entry_point || !name)
        return -EINVAL;

    multitasking::TaskManager& mngr = multitasking::TaskManager::instance();
    multitasking::Task* t = multitasking::TaskFactory::make_lightweight_task(mngr.get_current_task(), entry_point, name, multitasking::Task::DEFAULT_STACK_SIZE);
    t->set_arg1(arg);

    u32 task_id = mngr.add_task(t);
    if (task_id == 0)
        return -EPERM;

    return task_id;
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
    const string& cwd = current().task_group_data->cwd;

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
