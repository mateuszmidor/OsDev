/**
 *   @file: VfsRamFifoEntry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */


#include <errno.h>
#include "kstd.h"
#include "VfsRamFifoEntry.h"

namespace filesystem {

s64 VfsRamFifoEntry::read(void* data, u32 count) {
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

s64 VfsRamFifoEntry::write(const void* data, u32 count) {
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

} /* namespace filesystem */
