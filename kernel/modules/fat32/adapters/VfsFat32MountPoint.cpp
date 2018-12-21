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
namespace fat32 {

VfsFat32MountPoint::VfsFat32MountPoint(const VolumeFat32& volume) : volume(volume), root(volume.get_entry("/")), name(volume.get_label()) {
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 */
utils::SyscallResult<void> VfsFat32MountPoint::enumerate_entries(const OnVfsEntryFound& on_entry) {
    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        return on_entry(wrap_entry(e));
    };

    if (root.enumerate_entries(on_fat_entry) == Fat32EnumerateResult::ENUMERATION_FAILED)
        return {middlespace::ErrorCode::EC_NOTDIR};

    return {middlespace::ErrorCode::EC_OK};
}

utils::SyscallResult<VfsEntryPtr> VfsFat32MountPoint::get_entry(const UnixPath& unix_path) {
    if (auto e = volume.get_entry(unix_path))
        return {wrap_entry(e)};
    else
        return {middlespace::ErrorCode::EC_NOENT};
}

utils::SyscallResult<VfsEntryPtr> VfsFat32MountPoint::create_entry(const UnixPath& unix_path, bool is_directory) {
    if (auto e = volume.create_entry(unix_path, is_directory))
        return {wrap_entry(e)};
    else
        return {middlespace::ErrorCode::EC_INVAL};
}

utils::SyscallResult<void> VfsFat32MountPoint::delete_entry(const UnixPath& unix_path) {
    if (volume.delete_entry(unix_path))
        return {middlespace::ErrorCode::EC_OK};
    else
        return {middlespace::ErrorCode::EC_INVAL};
}

utils::SyscallResult<void> VfsFat32MountPoint::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    if (volume.move_entry(unix_path_from, unix_path_to))
        return {middlespace::ErrorCode::EC_OK};
    else
        return {middlespace::ErrorCode::EC_INVAL};
}

VfsEntryPtr VfsFat32MountPoint::wrap_entry(const Fat32Entry& e) const {
    if (e.is_directory())
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32DirectoryEntry>(e));
    else
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32FileEntry>(e));
}
} /* namespace fat32 */
} /* namespace filesystem */
