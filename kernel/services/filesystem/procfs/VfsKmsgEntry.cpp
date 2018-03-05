/**
 *   @file: kmsg.cpp
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */


#include <errno.h>
#include "kstd.h"
#include "KernelLog.h"
#include "procfs/VfsKmsgEntry.h"

using namespace logging;

namespace filesystem {

/**
 * @brief   Return length of kernel log
 */
utils::SyscallResult<u64> VfsKmsgEntry::get_size() const {
    return {KernelLog::instance().get_text().length()};
}

/**
 * @brief   Read the last "count" of kernel log bytes and clear the log
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsKmsgEntry::read(void* data, u32 count) {
    if (count == 0)
        return {0};

    KernelLog& klog = KernelLog::instance();
    const cstd::string& log = klog.get_text();
    if (log.empty())
        return {0};

    u32 read_start = max((s64)log.length() - count, 0);
    u32 num_bytes_to_read = min(count, log.length());

    memcpy(data, log.c_str() + read_start, num_bytes_to_read);

    klog.clear();
    return {num_bytes_to_read};
}

} /* namespace filesystem */
