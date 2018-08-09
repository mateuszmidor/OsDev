/**
 *   @file: OpenEntryTable.cpp
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntryTable.h"

using namespace cstd;

namespace filesystem {


/**
 * @brief   Prepare the entry table to work with dynamic memory once it is available in system
 */
void OpenEntryTable::install() {
    const u32 MAX_OPEN_ENTRIES {128};
    open_entries.resize(MAX_OPEN_ENTRIES);
}

/**
 * @brief   Put entry and it's state to table and return its file descriptor or empty if open file limit is reached
 */
Optional<GlobalFileDescriptor> OpenEntryTable::allocate(const VfsEntryPtr& e, EntryState* s) {
    if (auto fd = find_free_fd()) {
        open_entries[fd.value].entry = e;
        open_entries[fd.value].state = s;
        return {fd.value};
    }

    return {};
}

/**
 * @brief   Remove entry pointed by valid file descriptor from the table
 */
void OpenEntryTable::deallocate(GlobalFileDescriptor fd) {
    open_entries[fd].entry = nullptr;
    open_entries[fd].state = nullptr;
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
