/**
 *   @file: VfsManager.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMANAGER_H_
#define SRC_FILESYSTEM_VFSMANAGER_H_

#include "KernelLog.h"
#include "UnixPath.h"
#include "VfsEntry.h"
#include "kstd.h"

namespace filesystem {

class VfsManager {
public:
    static VfsManager& instance();
    VfsManager operator=(const VfsManager&) = delete;
    VfsManager operator=(VfsManager&&) = delete;
    void install_root();
    VfsEntry get_entry(const UnixPath& unix_path) const;

private:
    VfsManager() : klog(utils::KernelLog::instance()) {}
    VfsEntry get_entry_for_name(VfsEntry& parent_dir, const kstd::string& name) const;
    static VfsManager _instance;

    utils::KernelLog&       klog;
    VfsEntry*               root = nullptr;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMANAGER_H_ */
