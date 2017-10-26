/**
 *   @file: VfsFat32Entry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsFat32Entry.h"

namespace filesystem {

VfsFat32Entry::VfsFat32Entry(const Fat32Entry& e) : entry(e) {
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
    return entry.get_name();
}

s64 VfsFat32Entry::read(void* data, u32 count) {
    return entry.read(data, count);
}

s64 VfsFat32Entry::write(const void* data, u32 count) {
    return entry.write(data, count);
}

bool VfsFat32Entry::seek(u32 new_position) {
    return entry.seek(new_position);
}

bool VfsFat32Entry::truncate(u32 new_size) {
    return entry.truncate(new_size);
}

u32 VfsFat32Entry::get_position() const {
    return entry.get_position();
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
        VfsEntryPtr entry = std::make_shared<VfsFat32Entry>(e);
        return on_entry(entry);
    };

    if (entry.enumerate_entries(on_fat_entry) == EnumerateResult::ENUMERATION_STOPPED)
        return VfsEnumerateResult::ENUMERATION_STOPPED;

    return VfsEnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

} /* namespace filesystem */
