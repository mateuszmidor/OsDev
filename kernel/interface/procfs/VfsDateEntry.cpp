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

using namespace cstd;

namespace filesystem {


utils::SyscallResult<EntryState*> VfsDateEntry::open() {
    if (is_open)
        return {middlespace::ErrorCode::EC_AGAIN};

    is_open = true;
    return {nullptr};
}

utils::SyscallResult<void> VfsDateEntry::close(EntryState*) {
    is_open = false;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Read the last "count" bytes of date time string
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsDateEntry::read(EntryState*, void* data, u32 count) {
    if (!is_open)
        return {0};

    if (count == 0)
        return {0};

    const string info = get_date_time();
    if (info.empty())
        return {0};

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

string VfsDateEntry::get_date_time() const {
    u16 year    = read_byte(0x9);
    u16 month   = read_byte(0x8);
    u16 day     = read_byte(0x7);
    u16 hour    = read_byte(0x4);
    u16 minute  = read_byte(0x2);
    u16 second  = read_byte(0x0);

    return StringUtils::format("%-%-% %:%:% UTC\n", to_bin(year) + 2000, to_bin(month), to_bin(day), to_bin(hour), to_bin(minute), to_bin(second));
}

u8 VfsDateEntry::read_byte(u8 offset) const {
    address.write(offset);
    return data.read();
}

// convert bcd to bin
u8 VfsDateEntry::to_bin(u8 bcd) const {
    return (bcd / 16) * 10 + (bcd & 0xF);
}


} /* namespace filesystem */
