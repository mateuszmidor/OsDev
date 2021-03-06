/**
 *   @file: SysCallHandler.cpp
 *
 *   @date: Oct 22, 2017
 * @author: Mateusz Midor
 */

#include "SysCallHandler.h"

#include "cstd.h"
#include "String.h"
#include "Vector.h"
#include "ErrorCode.h"
#include "Vga.h"
#include "DriverManager.h"
#include "VgaDriver.h"
#include "VfsEntry.h"
#include "VfsManager.h"
#include "Task.h"
#include "TaskFactory.h"
#include "TaskGroupData.h"
#include "TimeManager.h"
#include "ElfRunner.h"
#include "SyscallResult.h"
#include "VfsRamFifoEntry.h"

using namespace cstd;
using namespace drivers;
using namespace filesystem;
using namespace ipc;

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
            -EINVAL if the "path" is not a valid path
  @note     As for now, flags and mode is unused
  @see      http://man7.org/linux/man-pages/man2/open.2.html
 */
s32 SysCallHandler::sys_open(const char path[], int flags, int mode) {
    auto& files = current().task_group_data->files;

    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {    // found empty file descriptor
            string absolute_path = make_absolute_path(path);
            auto open_result = VfsManager::instance().open(absolute_path);
            if (!open_result)
                return -(s32)open_result.ec;

            std::swap(files[i], open_result.value);
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

    files[fd] = {};
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

    if (auto read_result = files[fd]->read(buf, count))
        return read_result.value;
    else
        return -(s64)read_result.ec;
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

    if (auto write_result = files[fd]->write(buf, count))
        return write_result.value;
    else
        return -(s64)write_result.ec;
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
    case SEEK_SET: {
        new_position = offset;
        break;
    }

    case SEEK_CUR: {
        if (auto result = files[fd]->get_position())
            new_position = result.value + offset;
        else
            return -(off_t)result.ec;
        break;
    }

    case SEEK_END: {
        if (auto result = files[fd]->get_size())
            new_position = result.value + offset;
        else
            return -(off_t)result.ec;
        break;
    }

    default:
        return -EINVAL;
    }

    if (auto seek_result = files[fd]->seek(new_position))
        return new_position;
    else
        return -(off_t)seek_result.ec;
}

/**
 * @brief   Get file status
 * @return  0 on success
            -EINVAL if the "path" is not a valid path
 *          -ENOENT if the filesystem entry does not exist
 * @see     http://man7.org/linux/man-pages/man2/stat.2.html
 */
s32 SysCallHandler::sys_stat(const char path[], struct stat* buff) {
    string absolute_path = make_absolute_path(path);

    // first use entry_exists to prevent logging "no such entry exists" from get_entry
    auto exists_result = VfsManager::instance().exists(absolute_path);
    if (!exists_result)
        return -ENOENT;

    auto open_result = VfsManager::instance().open(absolute_path);
    if (!open_result)
        return -(s32)open_result.ec;

    memset(buff, 0, sizeof(struct stat));
    auto& entry = open_result.value;
    buff->st_size = entry->get_size().value;

    switch (entry->get_type()) {
    case VfsEntryType::DIRECTORY:
        buff->st_mode = S_IFDIR;
        break;

    case VfsEntryType::PIPE:
        buff->st_mode = S_IFIFO;
        break;

    default:
        buff->st_mode = S_IFREG;
        break;
    }

    return 0;
}

/**
 * @brief   Truncate file to given "length"
 * @return  0 on success
 *          -EINVAL if "length" is invalid or "path" is invalid
 *          -ENOENT if the filesystem entry does not exist
 *          -EISDIR if the filesystem entry is directory
 * @see     http://man7.org/linux/man-pages/man2/truncate.2.html
 */
s32 SysCallHandler::sys_truncate(const char path[], off_t length) {
    if (length < 0)
        return -EINVAL;

    string absolute_path = make_absolute_path(path);
    auto open_result = VfsManager::instance().open(absolute_path);
    if (!open_result)
        return -(s32)open_result.ec;

    auto& entry = open_result.value;
    if (entry->get_type() == VfsEntryType::DIRECTORY)
        return -EISDIR;

    if (auto trunc_result = entry->truncate(length))
        return 0;
    else
        return -(s32)trunc_result.ec;
}

/**
 * @brief   Move/rename filesystem entry
 * @return  0 on success
 *          -EINVAL if invalid path provided
 *          -ENOENT if source entry not exist
 * @see     http://man7.org/linux/man-pages/man2/rename.2.html
 */
s32 SysCallHandler::sys_rename(const char old_path[], const char new_path[]) {
    string absolute_old_path = make_absolute_path(old_path);
    string absolute_new_path = make_absolute_path(new_path);
    if (auto move_result = VfsManager::instance().move(absolute_old_path, absolute_new_path))
        return 0;
    else
        return -(s32)move_result.ec;
}

/**
 * @brief   Create directory
 * @return  0 on success
 *          -EINVAL if invalid path provided
 *          -EEXISTS if such entry already exists
 *          -EPERM if not allowed to create entry under given location
 * @see     http://man7.org/linux/man-pages/man2/mkdir.2.html
 */
s32 SysCallHandler::sys_mkdir(const char path[], int mode) {
    string absolute_path = make_absolute_path(path);
    if (auto create_result = VfsManager::instance().create(absolute_path, true))
        return 0;
    else
        return -(s32)create_result.ec;
}

/**
 * @brief   Remove directory which must be empty
 * @return  0 on success
 *          -EINVAL if invalid "path" provided
 *          -ENOENT if no such entry exists
 *          -ENOTDIR if filesystem entry is not directory
 * @see     http://man7.org/linux/man-pages/man2/rmdir.2.html
 */
s32 SysCallHandler::sys_rmdir(const char path[]) {
    string absolute_path = make_absolute_path(path);

    // RAII of open directory
    {
        auto open_result = VfsManager::instance().open(absolute_path);
        if (!open_result)
            return -(s32)open_result.ec;

        auto& entry = open_result.value;
        if (entry->get_type() != VfsEntryType::DIRECTORY)
            return -ENOTDIR;
    }

    if (auto remove_result = VfsManager::instance().remove(absolute_path))
        return 0;
    else
        return -(s32)remove_result.ec;
}

/**
 * @brief   Create file and return its descriptor
 * @param   mode NOT USED NOW
 * @return  Descriptor number on success
 *          -EINVAL if invalid "path" provided
 *          -EEXISTS if such entry already exists
 *          -EMFILE if per-process open file limit is reached
 * @see     http://man7.org/linux/man-pages/man2/open.2.html
 */
s32 SysCallHandler::sys_creat(const char path[], int mode) {
    string absolute_path = make_absolute_path(path);

    auto create_result = VfsManager::instance().create(absolute_path, false);
    if (!create_result)
        return -(s32)create_result.ec;

    // problem: created file may have different name eg. due to fat32 8.3 name restrictions
    auto open_result = VfsManager::instance().open(create_result.value);
    if (!open_result)
        return -(s32)open_result.ec;

    auto& entry = open_result.value;

    // alloc the created file in descriptor table
    auto& files = current().task_group_data->files;
    for (u32 i = 0; i < files.size(); i++)
        if (!files[i]) {    // found empty file descriptor
            std::swap(files[i], entry);
            return i;
        }

    return -EMFILE;
}

/**
 * @brief   Remove file from filesystem
 * @return  0 on success
 *          -EINVAL if invalid "path" provided
 *          -ENOENT if no such entry exists
 *          -EISDIR if filesystem entry is directory
 * @see     https://linux.die.net/man/2/unlink
 */
s32 SysCallHandler::sys_unlink(const char path[]) {
    string absolute_path = make_absolute_path(path);

    // RAII of open file
    {
        auto open_result = VfsManager::instance().open(absolute_path);
        if (!open_result)
            return -(s32)open_result.ec;

        auto& entry = open_result.value;
        if (entry->get_type() == VfsEntryType::DIRECTORY)
            return -EISDIR;
    }

    // RAII ensures the "path" entry is not open here by us
    if (auto remove_result = VfsManager::instance().remove(absolute_path))
        return 0;
    else
        return -(s32)remove_result.ec;
}

/**
 * @brief   Create regualar or special file
 * @param   path Path to be created
 * @param   mode Must be S_IFIFO (others to be implemented)
 * @param   dev Ignored
 * @return  0 on success
 * @see     http://man7.org/linux/man-pages/man2/mknod.2.html
 */
s32 SysCallHandler::sys_mknod(const char path[], int mode, int dev) {
    if (mode != S_IFIFO)
        return -EINVAL;

    UnixPath absolute_path = make_absolute_path(path);
    auto nod = cstd::make_shared<VfsRamFifoEntry>(absolute_path.extract_file_name());
    auto attach_result = VfsManager::instance().attach(absolute_path.extract_directory(), nod);
    return -(s32)attach_result.ec;
}
/**
 * @brief   Set new program break effectively changing the amount of dynamic memory available to a task
 * @return  New program break on success
 *          Current program break on failure (invalid argument or out of memory)
 * @see     http://man7.org/linux/man-pages/man2/brk.2.html
 */
u64 SysCallHandler::sys_brk(u64 new_brk) {
    multitasking::TaskManager& mngr = multitasking::TaskManager::instance();
    multitasking::TaskGroupDataPtr tgd = mngr.get_current_task().task_group_data;

    memory::AddressSpace& as = tgd->address_space;

    // invalid argument
    if (new_brk == 0)
        return as.heap_low_limit;

    // out of memory
    if (new_brk >= as.heap_high_limit)
        return as.heap_low_limit;

    // move program break
    as.heap_low_limit = new_brk;
    return new_brk;
}

/**
 * @brief   Get current working directory and store into "buff"
 * @param   size Size of the "buff"
 * @return  0 on success
 *          -EINVAL if "buff" is null
 *          -ERANGE if "size" < cwd.length + 1
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

    auto open_result = VfsManager::instance().open(absolute_path);
    if (!open_result)
        return -(s32)open_result.ec;

    auto& entry = open_result.value;
    if (entry->get_type() != VfsEntryType::DIRECTORY)
        return -ENOTDIR;

    current().task_group_data->cwd = absolute_path;
    return 0;
}

/**
 * @brief   Get time of specified "clk_id"
 * @return  0 on success
 *          -EINVAL if invalid/unsupported "clk_id" specified
 *          -EFAULT if invalid "tp" specified
 * @see     http://man7.org/linux/man-pages/man2/clock_gettime.2.html
 */
s32 SysCallHandler::sys_clock_gettime(clockid_t clk_id, struct timespec *tp) {
    if (!tp)
        return -EFAULT;

    if (clk_id != CLOCK_MONOTONIC)
        return -EINVAL;

    constexpr u64 NSEC = 1000*1000*1000;
    ktime::TimeManager& time_manager = ktime::TimeManager::instance();
    u64 ticks = time_manager.get_ticks();
    u64 freq = time_manager.get_hz();

    u64 sec = ticks / freq;
    tp->tv_sec = sec;
    tp->tv_nsec = ((ticks * NSEC) / freq) - (sec * NSEC);
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
 * @brief	Send signal to given task
 * @note	Only SIGKILL is supported for now
 * @see		http://man7.org/linux/man-pages/man2/kill.2.html
 */
//s32 SysCallHandler::sys_kill(u32 task_id, s32 signal) {
//	if (signal != SIGKILL)
//		return -EINVAL;
//	return -EPERM;//multitasking::Task::kill(task_id, signal);
//}

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
s32 SysCallHandler::enumerate(u32 fd, middlespace::VfsEntry* entries, u32 max_entries) {
    auto& files = current().task_group_data->files;

    if (fd >= files.size())
        return -EBADF;

    if (!files[fd])
        return -EBADF;

    if (files[fd]->get_type() != VfsEntryType::DIRECTORY)
        return -ENOTDIR;

    u32 entry_no = 0;

    OnVfsEntryFound on_entry = [&](const VfsEntryPtr& e) -> bool {
        if (entry_no > max_entries)
            return false;   // stop iterating

        entries[entry_no].is_directory = e->get_type() == VfsEntryType::DIRECTORY;
        entries[entry_no].size = e->get_size().value;

        // safely copy the entry name
        auto name_len = min(e->get_name().length(), sizeof(entries[entry_no].name) - 1);
        memcpy(entries[entry_no].name, e->get_name().c_str(), name_len);
        entries[entry_no].name[name_len] = '\0';

        entry_no++;

        return true;
    };

    if (files[fd]->enumerate_entries(on_entry))
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

u16 SysCallHandler::vga_get_char_at(u8 x, u8 y) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        return (u16)drv->char_at(x, y);
    }
    return 0;
}

void SysCallHandler::vga_set_char_at(u8 x, u8 y, u16 c) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        middlespace::VgaCharacter* vc = (middlespace::VgaCharacter*)&c;
        drv->char_at(x, y) = *vc;
    }
}

void SysCallHandler::vga_flush_char_buffer(const u16* buff) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->flush_char_buffer((middlespace::VgaCharacter*)buff);
    }
}

void SysCallHandler::vga_flush_video_buffer(const u8* buff) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->flush_video_buffer((middlespace::EgaColor64*)buff);
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
        drv->clear_screen(middlespace::EgaColor::Black);
    }
}

void SysCallHandler::vga_set_pixel_at(u16 x, u16 y, u8 c) {
    DriverManager& mngr = DriverManager::instance();
    if (VgaDriver* drv = mngr.get_driver<VgaDriver>()) {
        drv->put_pixel(x, y, (middlespace::EgaColor64)c);
    }
}

/**
 * @brief   Run ELF64 format program pointed by "path"
 * @param   nullterm_argv List of cstrings, last one == null
 */
s64 SysCallHandler::elf_run(const char path[], const char* nullterm_argv[]) {
    string absolute_path = make_absolute_path(path);

    auto open_result = VfsManager::instance().open(absolute_path);
    if (!open_result)
        return -(s64)open_result.ec; // no such file

    auto& entry = open_result.value;
    if (entry->get_type() != VfsEntryType::FILE)
        return -EISDIR;

    // TODO: reading elf file should be done from kernel task not from syscall, this is experimental version
    u32 size = entry->get_size().value;
    u8* elf_data = new u8[size];
    if (!elf_data)
        return -ENOMEM;

    auto read_result = entry->read(elf_data, size);
    if (!read_result)
        return -(s64)read_result.ec;

    vector<string> args;
    while (*nullterm_argv) {
        args.push_back(*nullterm_argv);
        nullterm_argv++;
    }

    elf64::ElfRunner runner;
    if (auto run_result = runner.run(elf_data, args))
        return run_result.value;
    else
        return -(s64)run_result.ec;
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
 *          -ENOMEM if out of memory
 */
s64 SysCallHandler::task_lightweight_run(u64 entry_point, u64 arg, const char name[]) {
    if (!entry_point || !name)
        return -EINVAL;

    multitasking::TaskManager& mngr = multitasking::TaskManager::instance();
    multitasking::Task* t = multitasking::TaskFactory::make_lightweight_task(mngr.get_current_task(), entry_point, name, multitasking::Task::DEFAULT_USER_STACK_SIZE);
    if (!t)
        return -ENOMEM;

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
UnixPath SysCallHandler::make_absolute_path(const string& path) const {
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
