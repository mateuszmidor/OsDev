/**
 *   @file: VfsPciInfoEntry.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "VfsPciInfoEntry.h"
#include "PCIController.h"

using namespace cstd;

namespace filesystem {

bool VfsPciInfoEntry::open() {
    if (is_open)
        return false;

    is_open = true;
    return true;
}

void VfsPciInfoEntry::close() {
    is_open = false;
}

/**
 * @brief   The size of the info is not known until the info string is built
 */
u32 VfsPciInfoEntry::get_size() const {
    return 0;
}

/**
 * @brief   Read the last "count" info bytes
 * @return  Num of read bytes
 */
s64 VfsPciInfoEntry::read(void* data, u32 count) {
    if (!is_open) {
        return 0;
    }

    if (count == 0)
        return 0;

    hardware::PCIController pcic;
    const string info = pcic.drivers_to_string();
    if (info.empty())
        return 0;

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close();
    return num_bytes_to_read;
}

s64 VfsPciInfoEntry::write(const void* data, u32 count) {
    return -EPERM;
}
} /* namespace filesystem */
