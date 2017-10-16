/*
 * VfsEntry.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: mateusz
 */

#include "VfsEntry.h"

namespace filesystem {

VfsEntry::VfsEntry() {
    // TODO Auto-generated constructor stub

}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    if (!is_dir){
//        klog.format("Fat32Entry::enumerate_entries: not a directory\n");
        return VfsEnumerateResult::ENUMERATION_FAILED;
    }

    for (VfsEntry* e : entries) {
        if (!on_entry(*e)) {
            return VfsEnumerateResult::ENUMERATION_STOPPED;
        }
    }

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

VfsEntry::operator bool() const {
    return !name.empty();
}

bool VfsEntry::operator!() const {
    return name.empty();
}

} /* namespace filesystem */
