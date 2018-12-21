/**
 *   @file: VfsCpuInfoEntry.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "VfsCpuInfoEntry.h"
#include "kstd.h"
#include "CpuInfo.h"
#include "StringUtils.h"
#include "CpuSpeedEstimator.h"

using namespace cstd;

namespace filesystem {

static string cpu_speed_in_mhz_or(const string& fallback_msg) {
    if (auto result = sysinfo::cpuspeedestimator::estimate_peak_mhz())
    	return StringUtils::from_int(result.value);

    return fallback_msg;
}

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

    hardware::CpuInfo cpu_info;
    const string info = StringUtils::format("CPU: % @ %MHz, %\n",
            cpu_info.get_vendor(),
            cpu_speed_in_mhz_or("<unknown>"),
            cpu_info.get_multimedia_extensions().to_string());

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

} /* namespace filesystem */
