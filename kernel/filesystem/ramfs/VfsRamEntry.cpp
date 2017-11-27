/**
 *   @file: VfsRamEntry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "VfsRamEntry.h"

namespace filesystem {

VfsRamEntry::VfsRamEntry(kstd::string name, bool is_dir) : name(name), is_dir(is_dir) {
}

VfsRamEntry::~VfsRamEntry() {
}

bool VfsRamEntry::is_directory() const {
    return is_dir;
}

u32 VfsRamEntry::get_size() const {
    return size;
}

s64 VfsRamEntry::read(void* data, u32 count) {
    if (is_dir)
        return -EISDIR;

    if (size == 0)
        return 0;

    if (count == 0)
        return 0;

    u32 num_bytes_to_read = kstd::min(size, count);
    u32 num_bytes_remaining = size - num_bytes_to_read;
    memcpy(data, buff, num_bytes_to_read);
    memcpy(buff, buff + num_bytes_to_read, num_bytes_remaining);
    size = num_bytes_remaining;

    return num_bytes_to_read;
}

s64 VfsRamEntry::write(const void* data, u32 count) {
    if (is_dir)
        return -EISDIR;

    // if buffer is full - block
    if (size == BUFF_SIZE)
        return -EWOULDBLOCK;

    if (count == 0)
        return 0;

    u32 remaining_space = BUFF_SIZE - size;
    u32 num_bytes_to_write = kstd::min(remaining_space, count);
    memcpy(buff + size, data, num_bytes_to_write);
    size += num_bytes_to_write;

    return num_bytes_to_write;
}

bool VfsRamEntry::seek(u32 new_position) {
    return false;
}

bool VfsRamEntry::truncate(u32 new_size) {
    return false;
}

const kstd::string& VfsRamEntry::get_name() const {
    return name;
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsRamEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
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
