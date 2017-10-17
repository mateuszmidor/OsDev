/**
 *   @file: VfsPseudoEntry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsPseudoEntry.h"

namespace filesystem {

VfsPseudoEntry::VfsPseudoEntry() {
}

VfsPseudoEntry::~VfsPseudoEntry() {
    for (VfsEntry* e : entries)
        delete e;
}

bool VfsPseudoEntry::is_directory() const {
    return is_dir;
}

u32 VfsPseudoEntry::get_size() const {
    return size;
}

const kstd::string& VfsPseudoEntry::get_name() const {
    return name;
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsPseudoEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    if (!is_dir)
        return VfsEnumerateResult::ENUMERATION_FAILED;

    for (VfsEntry* e : entries) {
        if (!on_entry(*e)) {
            return VfsEnumerateResult::ENUMERATION_STOPPED;
        }
    }

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

VfsEntry* VfsPseudoEntry::clone() const {
    VfsEntry* e = const_cast<VfsPseudoEntry*>(this);
    return e;
}
} /* namespace filesystem */
