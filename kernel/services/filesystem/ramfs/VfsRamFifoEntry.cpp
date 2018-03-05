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

/**
 * @brief   Read maximum of "count" bytes from the front of the pipe or block the reader if there is nothing to read
 */
utils::SyscallResult<u64> VfsRamFifoEntry::read(void* data, u32 count) {
    TaskManager& mngr = TaskManager::instance();

    // if buffer is empty - block the reader
    if (size == 0) {
        mngr.block_current_task(read_wait_list);
        return {middlespace::ErrorCode::EC_AGAIN};
    }

    // if requested zero bytes to read - return zero bytes read
    if (count == 0)
        return {0};

    // read the requested number of bytes
    u32 num_bytes_to_read = min(size, count);
    u32 num_bytes_remaining = size - num_bytes_to_read;
    memcpy(data, buff, num_bytes_to_read);

    // move the data in pipe towards the beginning.
    // This could be avoided by using circular buffer but this is not an issue for now
    // TODO: implement FIFO as circular buffer
    memmove(buff, buff + num_bytes_to_read, num_bytes_remaining);   // memmove is safe when buffers overlap
    size = num_bytes_remaining;

    // we made some room in the buffer - unblock the writers
    mngr.unblock_tasks(write_wait_list);

    return {num_bytes_to_read};
}

/**
 * @brief   Write maximum of "count" bytes to the end of the pipe or block the writer if there is no space left
 */
utils::SyscallResult<u64> VfsRamFifoEntry::write(const void* data, u32 count) {
    TaskManager& mngr = TaskManager::instance();

    // if buffer is full - block the writer
    if (size == BUFF_SIZE) {
        mngr.block_current_task(write_wait_list);
        return {middlespace::ErrorCode::EC_AGAIN};
    }

    // if requested zero bytes write - return zero bytes written
    if (count == 0)
        return {0};

    // write requested number of bytes
    u32 remaining_space = BUFF_SIZE - size;
    u32 num_bytes_to_write = min(remaining_space, count);
    memcpy(buff + size, data, num_bytes_to_write);
    size += num_bytes_to_write;

    // we got some data into the buffer - unblock the readers
    mngr.unblock_tasks(read_wait_list);

    return {num_bytes_to_write};
}

/**
 * @brief   Remove some older data from the pipe making room for new writes
 */
utils::SyscallResult<void> VfsRamFifoEntry::truncate(u32 new_size) {
    // enlarging fifo has no meaning
    if (new_size > size)
        return {middlespace::ErrorCode::EC_OK};

    // push the requested number of bytes into the void
    u32 num_bytes_to_skip = size - new_size;
    memmove(buff, buff + num_bytes_to_skip, new_size);  // memmove is safe when buffers overlap
    size = new_size;

    // we made some room in the buffer - unblock the writers
    TaskManager& mngr = TaskManager::instance();
    mngr.unblock_tasks(write_wait_list);

    return {middlespace::ErrorCode::EC_OK};
}

} /* namespace filesystem */
