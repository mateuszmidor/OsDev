/**
 *   @file: VfsDateEntry.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "VfsDateEntry.h"
#include "StringUtils.h"

namespace filesystem {


bool VfsDateEntry::open() {
    if (is_open)
        return false;

    is_open = true;
    return true;
}

void VfsDateEntry::close() {
    is_open = false;
}

/**
 * @brief   The size of date info is not known until the info string is built
 */
u32 VfsDateEntry::get_size() const {
    return 0;
}

/**
 * @brief   Read the last "count" date time bytes
 * @return  Num of read bytes
 */
s64 VfsDateEntry::read(void* data, u32 count) {
    if (!is_open) {
        return 0;
    }

    if (count == 0)
        return 0;

    const kstd::string info = get_date_time();
    if (info.empty())
        return 0;

    u32 read_start = kstd::max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = kstd::min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close();
    return num_bytes_to_read;
}

kstd::string VfsDateEntry::get_date_time() const {
    u16 year    = read_byte(0x9);
    u16 month   = read_byte(0x8);
    u16 day     = read_byte(0x7);
    u16 hour    = read_byte(0x4);
    u16 minute  = read_byte(0x2);
    u16 second  = read_byte(0x0);

    return kstd::StringUtils::format("%-%-% %:%:% UTC\n", bin(year) + 2000, bin(month), bin(day), bin(hour), bin(minute), bin(second));
}

u8 VfsDateEntry::read_byte(u8 offset) const {
    address.write(offset);
    return data.read();
}

// convert bcd to bin
u8 VfsDateEntry::bin(u8 bcd) const {
    return (bcd / 16) * 10 + (bcd & 0xF);
}

s64 VfsDateEntry::write(const void* data, u32 count) {
    return -EPERM;
}


} /* namespace filesystem */
