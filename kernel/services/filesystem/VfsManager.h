/**
 *   @file: VfsManager.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMANAGER_H_
#define SRC_FILESYSTEM_VFSMANAGER_H_

#include "VfsPersistentStorage.h"
#include "KernelLog.h"
#include "VfsRamStorage.h"

namespace filesystem {

class VfsManager {
public:
    static VfsManager& instance();
    VfsManager operator=(const VfsManager&) = delete;
    VfsManager operator=(VfsManager&&) = delete;
    void install();

    VfsEntryPtr get_entry(const UnixPath& unix_path);
    void close_entry(VfsEntryPtr& entry);
    VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory);
    bool delete_entry(const UnixPath& unix_path);
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to);
    bool copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to);
    VfsEntryPtr create_fifo(const UnixPath& unix_path);

private:
    VfsManager() : klog(logging::KernelLog::instance()) {}

    static VfsManager _instance;

    logging::KernelLog&         klog;
    VfsPersistentStorage        persistent_storage;
    VfsRamStorage               cache_storage;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMANAGER_H_ */
