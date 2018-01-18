/**
 *   @file: VfsFat32MountPoint.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsFat32Entry.h"
#include "VfsFat32MountPoint.h"

namespace filesystem {

VfsFat32MountPoint::VfsFat32MountPoint(const VolumeFat32& volume) : volume(volume), root(volume.get_entry("/")), name(volume.get_label()) {
}

VfsFat32MountPoint::~VfsFat32MountPoint() {
}

const cstd::string& VfsFat32MountPoint::get_name() const {
    return name;
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsFat32MountPoint::enumerate_entries(const OnVfsEntryFound& on_entry) {
    auto on_fat32_entry = [&](const Fat32Entry& e) -> bool {
        VfsEntryPtr entry = std::make_shared<VfsFat32Entry>(e);
        return on_entry(entry);
    };

    return (VfsEnumerateResult) root.enumerate_entries(on_fat32_entry);
}

VfsEntryPtr VfsFat32MountPoint::get_entry(const UnixPath& unix_path) {
    if (auto e = volume.get_entry(unix_path))
        return std::make_shared<VfsFat32Entry>(e);
    else
        return nullptr;
}

VfsEntryPtr VfsFat32MountPoint::create_entry(const UnixPath& unix_path, bool is_directory) const {
    if (auto e = volume.create_entry(unix_path, is_directory))
        return std::make_shared<VfsFat32Entry>(e);
    else
        return nullptr;
}

bool VfsFat32MountPoint::delete_entry(const UnixPath& unix_path) const {
    return volume.delete_entry(unix_path);
}

bool VfsFat32MountPoint::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    return volume.move_entry(unix_path_from, unix_path_to);
}

} /* namespace filesystem */
