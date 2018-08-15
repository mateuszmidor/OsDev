/**
 *   @file: VfsFat32MountPoint.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_
#define SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_

#include "VfsEntry.h"
#include "VolumeFat32.h"

namespace filesystem {

/**
 * @brief   This class is VfsMountPoint adapter for Fat32Volume, so Fat32Volume can be mounted in VFS
 */
class VfsFat32MountPoint: public VfsEntry {
public:
    VfsFat32MountPoint(const VolumeFat32& volume);

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
    VfsEntryPtr wrap_entry(const Fat32Entry& e) const;

    VolumeFat32         volume; // volume comes from MassStorageMsDos which got it from AtaDevice that is being held by AtaPrimaryBusDriver/AtaSecondaryBusDriver :)
    Fat32Entry          root;
    cstd::string        name;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_ */
