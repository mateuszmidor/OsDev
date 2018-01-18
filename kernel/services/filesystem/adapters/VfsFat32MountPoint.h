/**
 *   @file: VfsFat32MountPoint.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_
#define SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_

#include "VfsMountPoint.h"
#include "fat32/VolumeFat32.h"

namespace filesystem {

/**
 * @brief   This class is VfsMountPoint adapter for Fat32Volume, so Fat32Volume can be mounted in VFS
 */
class VfsFat32MountPoint: public VfsMountPoint {
public:
    VfsFat32MountPoint(const VolumeFat32& volume);
    virtual ~VfsFat32MountPoint();

    // [common interface]
    bool open() override                        { return true; /* no initialization to do here */ }
    void close() override                       {};
    const cstd::string& get_name() const;

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

    // [mount point interface]
    VfsEntryPtr get_entry(const UnixPath& unix_path) override;
    VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const override;
    bool delete_entry(const UnixPath& unix_path) const override;
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const override;

private:
    VolumeFat32         volume; // volume comes from MassStorageMsDos which got it from AtaDevice that is being held by AtaPrimaryBusDriver/AtaSecondaryBusDriver :)
    Fat32Entry          root;
    cstd::string        name;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_ */
