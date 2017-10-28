/**
 *   @file: kmsg.cpp
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */


#include <errno.h>
#include "KernelLog.h"
#include "procfs/VfsKmsgEntry.h"

using namespace utils;
namespace filesystem {

/**
 * @brief   Return length of kernel log
 */
u32 VfsKmsgEntry::get_size() const {
    return KernelLog::instance().get_text().length();
}

/**
 * @brief   Read the last "count" of kernel log bytes and clear the log
 * @return  Num of read bytes
 */
s64 VfsKmsgEntry::read(void* data, u32 count) {
    if (count == 0)
        return 0;

    KernelLog& klog = KernelLog::instance();
    const kstd::string& log = klog.get_text();
    if (log.empty())
        return 0;

    u32 read_start = kstd::max((s64)log.length() - count, 0);
    u32 num_bytes_to_read = kstd::min(count, log.length());

    memcpy(data, log.c_str() + read_start, num_bytes_to_read);

    klog.clear();
    return num_bytes_to_read;
}

s64 VfsKmsgEntry::write(const void* data, u32 count) {
    return -EPERM;
}
} /* namespace filesystem */
