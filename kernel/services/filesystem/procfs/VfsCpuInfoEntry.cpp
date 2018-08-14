/**
 *   @file: VfsCpuInfoEntry.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "StringUtils.h"
#include "VfsCpuInfoEntry.h"
#include "CpuInfo.h"

using namespace cstd;

namespace filesystem {

utils::SyscallResult<EntryState*> VfsCpuInfoEntry::open() {
    if (is_open)
        return {middlespace::ErrorCode::EC_AGAIN};

    is_open = true;
    return {nullptr};   // no state required
}

utils::SyscallResult<void> VfsCpuInfoEntry::close(EntryState*) {
    is_open = false;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Read the last "count" bytes of cpu info string
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsCpuInfoEntry::read(EntryState*, void* data, u32 count) {
    if (!is_open) {
        return {0};
    }

    if (count == 0)
        return {0};

    utils::CpuInfo cpu_info;
    const string info = StringUtils::format("CPU: % @ %MHz, %\n",
            cpu_info.get_vendor(),
            cpu_info.get_peak_mhz(),
            cpu_info.get_multimedia_extensions().to_string());

    if (info.empty())
        return {0};

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

} /* namespace filesystem */
