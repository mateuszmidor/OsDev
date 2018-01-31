/**
 *   @file: VfsFat32MountPoint.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsFat32FileEntry.h"
#include "VfsFat32DirectoryEntry.h"
#include "VfsFat32MountPoint.h"

namespace filesystem {

VfsFat32MountPoint::VfsFat32MountPoint(const VolumeFat32& volume) : volume(volume), root(volume.get_entry("/")), name(volume.get_label()) {
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsFat32MountPoint::enumerate_entries(const OnVfsEntryFound& on_entry) {
    // first enumerate the non-persistent entries like pipes, sockets, etc
    VfsDirectoryEntry::enumerate_entries(on_entry);

    // then enumerate the actual Fat32 directory entries
    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        return on_entry(wrap_entry(e));
    };

    if (root.enumerate_entries(on_fat_entry) == EnumerateResult::ENUMERATION_STOPPED)
        return VfsEnumerateResult::ENUMERATION_STOPPED;

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

VfsEntryPtr VfsFat32MountPoint::get_entry(const UnixPath& unix_path) {
    if (auto e = volume.get_entry(unix_path))
        return wrap_entry(e);
    else
        return {};
}

VfsEntryPtr VfsFat32MountPoint::create_entry(const UnixPath& unix_path, bool is_directory) const {
    if (auto e = volume.create_entry(unix_path, is_directory))
        return wrap_entry(e);
    else
        return {};
}

bool VfsFat32MountPoint::delete_entry(const UnixPath& unix_path) const {
    return volume.delete_entry(unix_path);
}

bool VfsFat32MountPoint::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    return volume.move_entry(unix_path_from, unix_path_to);
}

VfsEntryPtr VfsFat32MountPoint::wrap_entry(const Fat32Entry& e) const {
    if (e.is_directory())
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32DirectoryEntry>(e));
    else
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32FileEntry>(e));
}
} /* namespace filesystem */
