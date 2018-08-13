/**
 *   @file: VfsManager.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMANAGER_H_
#define SRC_FILESYSTEM_VFSMANAGER_H_

#include "KernelLog.h"
#include "SyscallResult.h"
#include "VfsCache.h"
#include "OpenEntry.h"
#include "VfsCachedEntry.h"
#include "VfsStorage.h"
#include "VfsTree.h"

namespace filesystem {

/**
 * @brief   This class provides and interface to the Virtual File System.
 */
class VfsManager {
public:
    static VfsManager& instance();
    VfsManager operator=(const VfsManager&) = delete;
    VfsManager operator=(VfsManager&&) = delete;

    void install_root();
    utils::SyscallResult<void> mount(const VfsEntryPtr& mount_point);
    utils::SyscallResult<void> attach_special(const UnixPath& path, const VfsEntryPtr& entry);

//    void close_entry(VfsEntryPtr& entry);
    utils::SyscallResult<void> entry_exists(const UnixPath& path) const;
    utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& path);
    utils::SyscallResult<VfsEntryPtr> create_entry(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> delete_entry(const UnixPath& path);
    utils::SyscallResult<void> move_entry(const UnixPath& path_from, const UnixPath& path_to);
//    bool copy_entry(const UnixPath& path_from, const UnixPath& path_to);

// NEW INTERFACE
public:
    utils::SyscallResult<OpenEntry> open(const UnixPath& path);
    utils::SyscallResult<void> attach(const VfsEntryPtr& entry, const UnixPath& path);
    utils::SyscallResult<void> create(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> remove(const UnixPath& path);
    utils::SyscallResult<void> copy(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move(const UnixPath& path_from, const UnixPath& path_to);
    bool exists(const UnixPath& path) const;

private:
    VfsManager() : klog(logging::KernelLog::instance()) {}
    utils::SyscallResult<VfsCachedEntryPtr> get_or_cache_directory(const UnixPath& path);

    static VfsManager   _instance;
    logging::KernelLog& klog;
    VfsStorage          storage;
    VfsCache            directory_cache;
    VfsTree             tree;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMANAGER_H_ */
