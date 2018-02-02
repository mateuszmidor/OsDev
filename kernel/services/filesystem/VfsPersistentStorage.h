/**
 *   @file: VfsPersistentStorage.h
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSPERSISTENTSTORAGE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSPERSISTENTSTORAGE_H_

#include "VfsRamDirectoryEntry.h"
#include "VfsMountPoint.h"
#include "AtaDriver.h"
#include "KernelLog.h"

namespace filesystem {

class VfsPersistentStorage {
public:
    VfsPersistentStorage() : klog(logging::KernelLog::instance()) {}
    void install();
    VfsEntryPtr get_entry(const UnixPath& unix_path) const;
    VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const;
    bool delete_entry(const UnixPath& unix_path) const;
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const;
    bool copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const;

private:
    void install_volumes(drivers::AtaDevice& hdd, VfsRamDirectoryEntryPtr parent);
    VfsMountPointPtr get_mountpoint_path(cstd::string& path);
    VfsEntryPtr get_entry_for_name(VfsEntryPtr parent_dir, const cstd::string& name) const;
    VfsMountPointPtr get_mountpoint_path(cstd::string& path) const;

    VfsRamDirectoryEntryPtr     root;
    logging::KernelLog&         klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSPERSISTENTSTORAGE_H_ */
