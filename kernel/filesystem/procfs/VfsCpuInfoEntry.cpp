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

namespace filesystem {


bool VfsCpuInfoEntry::open() {
    if (is_open)
        return false;

    is_open = true;
    return true;
}

void VfsCpuInfoEntry::close() {
    is_open = false;
}

/**
 * @brief   The size of date info is not known until the info string is built
 */
u32 VfsCpuInfoEntry::get_size() const {
    return 0;
}

/**
 * @brief   Read the last "count" date time bytes
 * @return  Num of read bytes
 */
s64 VfsCpuInfoEntry::read(void* data, u32 count) {
    if (!is_open) {
        return 0;
    }

    if (count == 0)
        return 0;

    utils::CpuInfo cpu_info;
    const kstd::string info = kstd::StringUtils::format("CPU: % @ %MHz, %\n",
            cpu_info.get_vendor(),
            cpu_info.get_peak_mhz(),
            cpu_info.get_multimedia_extensions().to_string());

    if (info.empty())
        return 0;

    u32 read_start = kstd::max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = kstd::min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close();
    return num_bytes_to_read;
}

s64 VfsCpuInfoEntry::write(const void* data, u32 count) {
    return -EPERM;
}
} /* namespace filesystem */
