/**
 *   @file: VfsFat32Entry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsFat32Entry.h"

namespace filesystem {

VfsFat32Entry::VfsFat32Entry(const Fat32Entry& e) : entry(e), name(e.get_name()) {
}

VfsFat32Entry::~VfsFat32Entry() {
}

bool VfsFat32Entry::is_directory() const {
    return entry.is_directory();
}

u32 VfsFat32Entry::get_size() const {
    return entry.get_size();
}

const kstd::string& VfsFat32Entry::get_name() const {
    return name;
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsFat32Entry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    if (!is_directory())
        return VfsEnumerateResult::ENUMERATION_FAILED;

    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        VfsFat32Entry entry(e);
        return on_entry(entry);
    };

    if (entry.enumerate_entries(on_fat_entry) == EnumerateResult::ENUMERATION_STOPPED)
        return VfsEnumerateResult::ENUMERATION_STOPPED;

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

VfsEntry* VfsFat32Entry::clone() const {
    return new VfsFat32Entry(*this);
}

void VfsFat32Entry::set_custom_name(const kstd::string& name) {
    this->name = name;
}
} /* namespace filesystem */
