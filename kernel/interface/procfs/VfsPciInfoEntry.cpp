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

utils::SyscallResult<EntryState*> VfsPciInfoEntry::open() {
    if (is_open)
        return {middlespace::ErrorCode::EC_AGAIN};

    is_open = true;
    return {nullptr};
}

utils::SyscallResult<void> VfsPciInfoEntry::close(EntryState*) {
    is_open = false;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Read the last "count" bytes of PCI info string
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsPciInfoEntry::read(EntryState*, void* data, u32 count) {
    if (!is_open) {
        return {0};
    }

    if (count == 0)
        return {0};

    hardware::PCIController pcic;
    const string info = pcic.drivers_to_string();
    if (info.empty())
        return {0};

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

} /* namespace filesystem */
