/**
 *   @file: VfsMemInfoEntry.cpp
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "StringUtils.h"
#include "VfsMemInfoEntry.h"
#include "MemoryManager.h"
#include "FrameAllocator.h"

using namespace cstd;
using namespace memory;

namespace filesystem {

utils::SyscallResult<EntryState*> VfsMemInfoEntry::open() {
    if (is_open)
        return {middlespace::ErrorCode::EC_AGAIN};

    is_open = true;
    return {nullptr};
}

utils::SyscallResult<void> VfsMemInfoEntry::close(EntryState*) {
    is_open = false;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Read the last "count" of kernel log bytes and clear the log
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsMemInfoEntry::read(void* data, u32 count) {
    if (!is_open)
        return {0};

    if (count == 0)
        return {0};

    const string info = get_info();
    if (info.empty())
        return {0};

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

string VfsMemInfoEntry::get_info() const {
//    MemoryManager& mm = MemoryManager::instance();
//    size_t free_memory = mm.get_total_memory_in_bytes() - mm.get_free_memory_in_bytes();
//    size_t total_memory = mm.get_total_memory_in_bytes();
//    env->printer->format("Used memory so far: % KB, total available: % MB\n", free_memory / 1024, total_memory / 1024 / 1024);

    size_t used_frames = FrameAllocator::get_used_frames_count();
    size_t total_frames = FrameAllocator::get_total_frames_count();
    return StringUtils::format("Used frames so far: %, total available: %\n", used_frames, total_frames);
}


} /* namespace filesystem */
