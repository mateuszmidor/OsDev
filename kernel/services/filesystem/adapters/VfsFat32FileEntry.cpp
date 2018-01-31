/**
 *   @file: VfsFat32FileEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include "VfsFat32FileEntry.h"

namespace filesystem {

VfsFat32FileEntry::VfsFat32FileEntry(const Fat32Entry& e) : entry(e) {
}

u32 VfsFat32FileEntry::get_size() const {
    return entry.get_size();
}

const cstd::string& VfsFat32FileEntry::get_name() const {
    return entry.get_name();
}

s64 VfsFat32FileEntry::read(void* data, u32 count) {
    return entry.read(data, count);
}

s64 VfsFat32FileEntry::write(const void* data, u32 count) {
    return entry.write(data, count);
}

bool VfsFat32FileEntry::seek(u32 new_position) {
    return entry.seek(new_position);
}

bool VfsFat32FileEntry::truncate(u32 new_size) {
    return entry.truncate(new_size);
}

u32 VfsFat32FileEntry::get_position() const {
    return entry.get_position();
}
} /* namespace filesystem */
