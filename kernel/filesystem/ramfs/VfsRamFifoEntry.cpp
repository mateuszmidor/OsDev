/**
 *   @file: VfsRamFifoEntry.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */


#include <errno.h>
#include "kstd.h"
#include "VfsRamFifoEntry.h"


using namespace multitasking;
namespace filesystem {

s64 VfsRamFifoEntry::read(void* data, u32 count) {
    TaskManager& mngr = TaskManager::instance();

    // if buffer is empty - block the reader
    if (size == 0) {
        mngr.block_current_task(read_wait_list);
        return -EWOULDBLOCK;
    }

    if (count == 0)
        return 0;

    u32 num_bytes_to_read = kstd::min(size, count);
    u32 num_bytes_remaining = size - num_bytes_to_read;
    memcpy(data, buff, num_bytes_to_read);
    memcpy(buff, buff + num_bytes_to_read, num_bytes_remaining);
    size = num_bytes_remaining;

    // data got out of the buffer - unblock the writers
    mngr.unblock_tasks(write_wait_list);

    return num_bytes_to_read;
}

s64 VfsRamFifoEntry::write(const void* data, u32 count) {
    TaskManager& mngr = TaskManager::instance();

    // if buffer is full - block the writer
    if (size == BUFF_SIZE) {
        mngr.block_current_task(write_wait_list);
        return -EWOULDBLOCK;
    }

    if (count == 0)
        return 0;

    u32 remaining_space = BUFF_SIZE - size;
    u32 num_bytes_to_write = kstd::min(remaining_space, count);
    memcpy(buff + size, data, num_bytes_to_write);
    size += num_bytes_to_write;

    // data got into the buffer - unblock the readers
    mngr.unblock_tasks(read_wait_list);

    return num_bytes_to_write;
}

bool VfsRamFifoEntry::truncate(u32 new_size) {
    // cant resize the file beyond the buffer size
    if (new_size > BUFF_SIZE)
        return false;

    size = new_size;

    // buffer clear - unblock the writers
    TaskManager& mngr = TaskManager::instance();
    mngr.unblock_tasks(write_wait_list);

    return true;
}

} /* namespace filesystem */
