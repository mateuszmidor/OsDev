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
 * @brief   This class is a Virtual File System mount point interface. Such a mount point manages entire subtree, eg entire filesystem volume
 */
class VfsMountPoint: public VfsEntry {
public:

    // [common interface]
    bool is_directory() const override          { return true; }
    bool is_mount_point() const override        { return true; }

    // [file interface - not applicable for mountpoint]
    u32 get_size() const override               { return 0; }
    u32 read(void* data, u32 count) override    { return 0; }
    u32 write(const void* data, u32 count)      { return 0; }
    bool seek(u32 new_position) override        { return false; }
    bool truncate(u32 new_size) override        { return false; };

    // [volume interface]
    virtual VfsEntryPtr get_entry(const UnixPath& unix_path) = 0;
    virtual VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const = 0;
    virtual bool delete_entry(const UnixPath& unix_path) const = 0;
    virtual bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const = 0;
};

using VfsMountPointPtr = std::shared_ptr<VfsMountPoint>;

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSMOUNTPOINT_H_ */
