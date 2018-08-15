/**
 *   @file: VfsFat32FileEntry.cpp
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#include "VfsFat32FileEntry.h"

namespace filesystem {

VfsFat32FileEntry::VfsFat32FileEntry(const Fat32Entry& e) : entry(e) {
}


utils::SyscallResult<void> VfsFat32FileEntry::seek(EntryState* state, u32 new_position) {
    if (entry.seek(*(Fat32State*)state, new_position))
        return {middlespace::ErrorCode::EC_OK};
    else
        return {middlespace::ErrorCode::EC_INVAL};
}

utils::SyscallResult<void> VfsFat32FileEntry::truncate(EntryState* state, u32 new_size) {
    if (entry.truncate(*(Fat32State*)state, new_size))
        return {middlespace::ErrorCode::EC_OK};
    else
        return {middlespace::ErrorCode::EC_INVAL};
}

} /* namespace filesystem */
