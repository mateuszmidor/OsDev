/**
 *   @file: VfsMountPoint.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSMOUNTPOINT_H_
#define SRC_FILESYSTEM_VFSMOUNTPOINT_H_

#include "VfsEntry.h"
#include "UnixPath.h"

namespace filesystem {

/**
 * @brief   This class is a Virtual File System mount point interface
 */
class VfsMountPoint: public VfsEntry {
public:
    bool is_mount_point() override { return true; }

    // [volume interface]
    virtual VfsEntryPtr get_entry(const UnixPath& unix_path) = 0;
    virtual VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const = 0;
    virtual bool delete_entry(const UnixPath& unix_path) const = 0;
    virtual bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const = 0;
};

using VfsMountPointPtr = std::shared_ptr<VfsMountPoint>;

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMOUNTPOINT_H_ */
