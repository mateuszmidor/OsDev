/**
 *   @file: VfsManager.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMANAGER_H_
#define SRC_FILESYSTEM_VFSMANAGER_H_

#include "AtaDriver.h"
#include "VolumeFat32.h"
#include "KernelLog.h"
#include "UnixPath.h"
#include "VfsPseudoEntry.h"
#include "kstd.h"

namespace filesystem {

class VfsManager {
public:
    static VfsManager& instance();
    VfsManager operator=(const VfsManager&) = delete;
    VfsManager operator=(VfsManager&&) = delete;
    void install_root();

    VfsEntryPtr get_entry(const UnixPath& unix_path);
    VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const;
    bool delete_entry(const UnixPath& unix_path) const;
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const;

private:
    VfsManager() : klog(utils::KernelLog::instance()) {}
    VfsEntryPtr get_entry_for_name(VfsEntryPtr parent_dir, const kstd::string& name);

    void install_ata_devices();
    void install_volumes(drivers::AtaDevice& hdd);
    static VfsManager _instance;

    utils::KernelLog&       klog;
    VfsEntryPtr             root;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMANAGER_H_ */
