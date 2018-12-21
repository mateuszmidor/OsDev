/**
 *   @file: VfsFat32DirectoryEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include "VfsFat32DirectoryEntry.h"
#include "VfsFat32FileEntry.h"

namespace filesystem {
namespace fat32 {


/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 */
utils::SyscallResult<void> VfsFat32DirectoryEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    // then enumerate the actual Fat32 directory entries
    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        return on_entry(wrap_entry(e));
    };

    if (entry.enumerate_entries(on_fat_entry) == Fat32EnumerateResult::ENUMERATION_FAILED)
        return {middlespace::ErrorCode::EC_NOTDIR};

    return {middlespace::ErrorCode::EC_OK};
}

utils::SyscallResult<VfsEntryPtr> VfsFat32DirectoryEntry::get_entry(const UnixPath& unix_path) {
    VfsEntryPtr result;
    auto on_entry = [&](VfsEntryPtr e) -> bool {
        if (e->get_name() == (cstd::string)unix_path) {
            result = e;
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };

    if (result)
        return {result};
    else
        return {middlespace::ErrorCode::EC_NOENT};
}

VfsEntryPtr VfsFat32DirectoryEntry::wrap_entry(const Fat32Entry& e) const {
    if (e.is_directory())
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32DirectoryEntry>(e));
    else
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32FileEntry>(e));
}

} /* namespace fat32 */
} /* namespace filesystem */
