/**
 *   @file: VfsPseudoEntry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsPseudoEntry.h"

namespace filesystem {

VfsPseudoEntry::VfsPseudoEntry(kstd::string name, bool is_dir) : name(name), is_dir(is_dir) {
}

VfsPseudoEntry::~VfsPseudoEntry() {
}

bool VfsPseudoEntry::is_directory() const {
    return is_dir;
}

u32 VfsPseudoEntry::get_size() const {
    return size;
}

u32 VfsPseudoEntry::read(void* data, u32 count) {
    return 0;
}

u32 VfsPseudoEntry::write(const void* data, u32 count) {
    return 0;
}

bool VfsPseudoEntry::seek(u32 new_position) {
    return false;
}

bool VfsPseudoEntry::truncate(u32 new_size) {
    return false;
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

    for (VfsEntryPtr e : entries) {
        if (!on_entry(e)) {
            return VfsEnumerateResult::ENUMERATION_STOPPED;
        }
    }

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

} /* namespace filesystem */
