/**
 *   @file: VfsFat32DirectoryEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include "VfsFat32DirectoryEntry.h"
#include "VfsFat32FileEntry.h"

namespace filesystem {


/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsFat32DirectoryEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    // first enumerate the non-persistent entries like pipes, sockets, etc
    VfsDirectoryEntry::enumerate_entries(on_entry);

    // then enumerate the actual Fat32 directory entries
    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        return on_entry(wrap_entry(e));
    };

    if (entry.enumerate_entries(on_fat_entry) == EnumerateResult::ENUMERATION_STOPPED)
        return VfsEnumerateResult::ENUMERATION_STOPPED;

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

VfsEntryPtr VfsFat32DirectoryEntry::wrap_entry(const Fat32Entry& e) const {
    if (e.is_directory())
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32DirectoryEntry>(e));
    else
        return std::static_pointer_cast<VfsEntry>(std::make_shared<VfsFat32FileEntry>(e));
}

} /* namespace filesystem */
