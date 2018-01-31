/**
 *   @file: VfsDirectoryEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include "VfsDirectoryEntry.h"

namespace filesystem {

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsDirectoryEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    for (VfsEntryPtr e : nonpersistent_entries) {
        if (!on_entry(e)) {
            return VfsEnumerateResult::ENUMERATION_STOPPED;
        }
    }

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}
} /* namespace filesystem */
