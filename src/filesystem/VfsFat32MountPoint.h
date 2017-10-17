/**
 *   @file: VfsFat32MountPoint.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_
#define SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_

#include "VfsMountPoint.h"
#include "VolumeFat32.h"

namespace filesystem {

class VfsFat32MountPoint: public VfsMountPoint {
public:
    VfsFat32MountPoint(const VolumeFat32& volume);
    virtual ~VfsFat32MountPoint();

    // [common interface]
    bool is_directory() const override;
    const kstd::string& get_name() const;

    // [file interface]
    u32 get_size() const override;
    u32 read(void* data, u32 count) override;
    u32 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override;
    bool truncate(u32 new_size) override;

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

    // [mount point interface]
    VfsEntryPtr get_entry(const UnixPath& unix_path) override;
    VfsEntryPtr create_entry(const UnixPath& unix_path, bool is_directory) const override;
    bool delete_entry(const UnixPath& unix_path) const override;
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const override;

private:
    const VolumeFat32&    volume; // volume comes from
    Fat32Entry      root;
    kstd::string    name;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32MOUNTPOINT_H_ */
