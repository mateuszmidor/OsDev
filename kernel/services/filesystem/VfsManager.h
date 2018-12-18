/**
 *   @file: VfsManager.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMANAGER_H_
#define SRC_FILESYSTEM_VFSMANAGER_H_

#include "VfsTree.h"
#include "OpenEntry.h"

namespace filesystem {

/**
 * @brief   This class provides and interface to the Virtual File System.
 */
class VfsManager {
public:
    static VfsManager& instance();
    VfsManager operator=(const VfsManager&) = delete;
    VfsManager operator=(VfsManager&&) = delete;

    void install();
    utils::SyscallResult<OpenEntryPtr> open(const UnixPath& path);
    utils::SyscallResult<void> attach(const UnixPath& path, const VfsEntryPtr& entry);
    utils::SyscallResult<UnixPath> create(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> remove(const UnixPath& path);
    utils::SyscallResult<void> copy(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move(const UnixPath& path_from, const UnixPath& path_to);
    bool exists(const UnixPath& path) const;

private:
    static VfsManager   _instance;
    VfsTree             tree;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMANAGER_H_ */
