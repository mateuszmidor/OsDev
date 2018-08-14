/**
 *   @file: VfsRamDirectoryEntry.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "VfsRamDirectoryEntry.h"

namespace filesystem {

utils::SyscallResult<void> VfsRamDirectoryEntry::set_name(const cstd::string& name) {
    this->name = name;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Get entry for given name, or error code if does not exist
 */
utils::SyscallResult<VfsEntryPtr> VfsRamDirectoryEntry::get_entry(const UnixPath& name) {
    auto it = find_entry(name);
    if (it != entries.end())
        return {*it};

    // no such entry
    return {middlespace::ErrorCode::EC_NOENT};
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 */
utils::SyscallResult<void> VfsRamDirectoryEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    for (const VfsEntryPtr& e : entries)
        if (!on_entry(e))
            break;

    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Attach an "entry" to the directory
 */
bool VfsRamDirectoryEntry::attach_entry(const VfsEntryPtr& entry) {
    if (find_entry(entry->get_name()) != entries.end())
        return false;

    entries.push_back(entry);
    return true;
}

/**
 * @brief   Detach entry of given "name"
 */
bool VfsRamDirectoryEntry::detach_entry(const cstd::string& name) {
    auto it = find_entry(name);
    if (it == entries.end())
        return false;

    entries.erase(it);
    return true;
}

/**
 * @brief   Check if directory holds no contents
 */
bool VfsRamDirectoryEntry::is_empty() const {
    return entries.empty();
}

/**
 * @brief   Find iterator of non-persistent entry of given name
 */
cstd::vector<VfsEntryPtr>::iterator VfsRamDirectoryEntry::find_entry(const cstd::string& name) {
    auto is_same_name = [&name](const VfsEntryPtr& e) {
        return (e->get_name() == name);
    };

    // find entry of given name and erase it
    return std::find_if(entries.begin(), entries.end(), is_same_name);
}
} /* namespace filesystem */
