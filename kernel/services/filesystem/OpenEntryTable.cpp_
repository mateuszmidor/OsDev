/**
 *   @file: OpenEntryTable.cpp
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntryTable.h"

using namespace cstd;
using namespace utils;
using namespace middlespace;

namespace filesystem {


/**
 * @brief   Prepare the entry table to work with dynamic memory once it is available in system
 */
void OpenEntryTable::install() {
    const u32 MAX_OPEN_ENTRIES {128};
    open_entries.resize(MAX_OPEN_ENTRIES);
}

/**
 * @brief   Open entry, allocate it in table and return it's file descriptor, or error code on error
 */
utils::SyscallResult<GlobalFileDescriptor> OpenEntryTable::open(const VfsCachedEntryPtr& e) {
    auto fd = find_free_fd();
    if (!fd) {
        klog.format("OpenEntryTable::open: cannot open entry; open entry limit reached:  %\n", open_entries.size());
        return {ErrorCode::EC_MFILE};
    }

    auto open_result = e->open();
    if (!open_result) {
        klog.format("OpenEntryTable::open: cannot open entry; entry refused being open\n");
        return {open_result.ec};
    }

    e->open_count++;
    open_entries[fd.value].entry = e;
    open_entries[fd.value].state = open_result.value;
    return {fd.value};
}

/**
 * @brief   Remove entry from the table, return the entry itself
 */
SyscallResult<VfsCachedEntryPtr> OpenEntryTable::close(GlobalFileDescriptor fd) {
    if (!is_open(fd)) {
        klog.format("OpenEntryTable::close: cannot close entry; entry not open: fd %\n", fd);
        return {ErrorCode::EC_BADF};
    }

    auto& open_entry = open_entries[fd];
    auto close_result = open_entry.entry->close(open_entry.state);
    if (!close_result) {
        klog.format("OpenEntryTable::close: cannot close entry; entry refused closing: fd %\n", fd);
        return {close_result.ec};
    }

    VfsCachedEntryPtr entry;
    std::swap(open_entries[fd].entry, entry);
    open_entries[fd].state = nullptr;
    entry->open_count--;

    return {entry};
}

/**
 * @@brief  Check if given filedescriptor points to a valid open entry
 */
bool OpenEntryTable::is_open(GlobalFileDescriptor fd) const {
    if (fd >= open_entries.size())
        return false;

    if (!open_entries[fd].entry || !open_entries[fd].state)
        return false;

    return true;
}

/**
 * @brief   Unchecked access method for open entries
 */
OpenEntry& OpenEntryTable::operator[](GlobalFileDescriptor fd) {
    return open_entries[fd];
}

const OpenEntry& OpenEntryTable::operator[](GlobalFileDescriptor fd) const {
    return open_entries[fd];
}

/**
 * @brief  Return unused file descriptor if available, empty otherwise
 */
Optional<GlobalFileDescriptor> OpenEntryTable::find_free_fd() const {
    for (u32 i = 0; i < open_entries.size(); i++)
        if (!open_entries[i].entry)
            return {i};

    return {};
}
} /* namespace filesystem */
