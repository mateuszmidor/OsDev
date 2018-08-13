/**
 *   @file: OpenEntry.cpp
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntry.h"
#include "VfsManager.h"

using namespace middlespace;

namespace filesystem {

OpenEntry::~OpenEntry() {
    entry->open_count--;
    VfsManager::instance().release(entry);
}

utils::SyscallResult<OpenEntryPtr> OpenEntry::open(const UnixPath& path) {
    auto entry = VfsManager::instance().get(path);
    if (!entry)
        return {ErrorCode::EC_NOENT};

    auto open_result = entry->open();
    if (!open_result)
        return {open_result.ec};

    entry->open_count++;

    return {OpenEntryPtr(new OpenEntry(entry, open_result.value))};
}
} /* namespace filesystem */
