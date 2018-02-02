/**
 *   @file: VfsDirectoryEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "VfsDirectoryEntry.h"

namespace filesystem {

/**
 * @brief   Detach non-peristent entry of given name
 */
void VfsDirectoryEntry::detach_entry(const cstd::string& name) {
    auto is_same_name = [&name](VfsEntryPtr e) {
        return (e->get_name() == name);
    };

    auto it = std::find_if(nonpersistent_entries.begin(), nonpersistent_entries.end(), is_same_name);
    if (it != nonpersistent_entries.end())
        nonpersistent_entries.erase(it);
}

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
