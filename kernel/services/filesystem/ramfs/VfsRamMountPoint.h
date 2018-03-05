/**
 *   @file: VfsRamMountPoint.h
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMMOUNTPOINT_H_
#define KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMMOUNTPOINT_H_

#include "VfsRamDirectoryEntry.h"
#include "KernelLog.h"

namespace filesystem {

/**
 * @brief   This class represents an in-memory filesystem, to be used eg. for userspace-kernelspace communication; /proc, /dev
 */
class VfsRamMountPoint: public VfsEntry {
public:
    VfsRamMountPoint(const cstd::string& name);

    // [common interface]
    const cstd::string& get_name() const override   { return name; }
    VfsEntryType get_type() const override          { return VfsEntryType::DIRECTORY; }
    bool is_mountpoint() const override             { return true; }

    // [directory interface]
    utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& path) override;
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) override;

    // [mount point interface]
    utils::SyscallResult<VfsEntryPtr> create_entry(const UnixPath& path, bool is_directory) override;
    utils::SyscallResult<void> delete_entry(const UnixPath& path) override;
    utils::SyscallResult<void> move_entry(const UnixPath& from, const UnixPath& to) override;

private:
    bool is_non_empty_directory(const VfsEntryPtr& e) const;

    const cstd::string          name;
    VfsRamDirectoryEntryPtr     root;
    logging::KernelLog&         klog;
};

}  /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMMOUNTPOINT_H_ */
