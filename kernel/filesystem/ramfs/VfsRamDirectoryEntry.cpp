/**
 *   @file: VfsRamDirectoryEntry.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "VfsRamDirectoryEntry.h"

namespace filesystem {

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsRamDirectoryEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    for (VfsEntryPtr e : entries) {
        if (!on_entry(e)) {
            return VfsEnumerateResult::ENUMERATION_STOPPED;
        }
    }

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

} /* namespace filesystem */
