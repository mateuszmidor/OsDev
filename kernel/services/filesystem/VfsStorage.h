/**
 *   @file: VfsStorage.h
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSSTORAGE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSSTORAGE_H_

#include "VfsRamDirectoryEntry.h"
#include "KernelLog.h"

namespace filesystem {

namespace details {

/**
 * @brief   This class holds a mountpoint vfs entry and a path to an element under that mountpoint
 */
struct MountpointPath {
    VfsEntryPtr     mountpoint;
    cstd::string    path;
    operator bool() { return (bool)mountpoint; }
    bool operator!(){ return !mountpoint; }
};
}

/**
 * @brief   This class holds the virtual file system root and exposes all the necessary methods to access and modify the vfs
 */
class VfsStorage {
public:
    VfsStorage() : klog(logging::KernelLog::instance()) {}
    void install();
    VfsRamDirectoryEntryPtr& get_root() { return root; }
    bool entry_exists(const UnixPath& path) const;
    VfsEntryPtr get_entry(const UnixPath& path) const;
    VfsEntryPtr create_entry(const UnixPath& path, bool is_directory) const;
    utils::SyscallResult<void> delete_entry(const UnixPath& path) const;
    bool move_entry(const UnixPath& path_from, const UnixPath& path_to) const;
    bool copy_entry(const UnixPath& path_from, const UnixPath& path_to) const;

private:
    VfsEntryPtr get_entry_for_name(VfsEntryPtr parent_dir, const cstd::string& name) const;
    details::MountpointPath get_mountpoint_path(const cstd::string& path) const;

    VfsRamDirectoryEntryPtr     root;
    logging::KernelLog&         klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSSTORAGE_H_ */
