/**
 *   @file: VfsEntryAttachmentDecorator.cpp
 *
 *   @date: Feb 15, 2018
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "VfsCachedEntry.h"

namespace filesystem {

/**
 * @brief   Get entry for given name, or empty pointer if not exists
 */
utils::SyscallResult<VfsEntryPtr> VfsCachedEntry::get_entry(const UnixPath& path) {
    // lookup entry in attachments
    if (auto result = get_attached_entry(path))
        return {result};

    // lookup entry in decorated entry
    if (auto result = e->get_entry(path))
        return result;

    // no such entry
    return {middlespace::ErrorCode::EC_NOENT};
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  EC_OK when all entries have been enumerated,
 */
utils::SyscallResult<void> VfsCachedEntry::enumerate_entries(const OnVfsEntryFound& on_entry) {
    // enumerate attachments
    for (const auto& e : attached_entries)
        if (!on_entry(e))
            return {middlespace::ErrorCode::EC_OK};

    // enumerate decorated entry
    e->enumerate_entries(on_entry);

    // if no entries enumerated - it is also fine result eg for empty directory
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Attach an entry
 */
bool VfsCachedEntry::attach_entry(const VfsEntryPtr& entry) {
    if (find_attached_entry(entry->get_name()) != attached_entries.end())
        return false;

    attached_entries.push_back(std::make_shared<VfsCachedEntry>(entry));
    return true;
}

/**
 * @brief   Detach entry of given name
 */
bool VfsCachedEntry::detach_entry(const cstd::string& name) {
    const auto it = find_attached_entry(name);
    if (it == attached_entries.end())
        return false;

    attached_entries.erase(it);
    return true;
}

/**
 * @brief   Lookup entry of given "name" on the attachments list
 */
VfsCachedEntryPtr VfsCachedEntry::get_attached_entry(const cstd::string& name) {
    auto it = find_attached_entry(name);
    if (it != attached_entries.end())
        return {*it};

    // no such entry
    return {};
}

/**
 * @brief   Find iterator of attached entry of given "name"
 */
cstd::vector<VfsCachedEntryPtr>::iterator VfsCachedEntry::find_attached_entry(const cstd::string& name) {
    auto is_same_name = [&name](const VfsCachedEntryPtr& e) {
        return (e->get_name() == name);
    };

    // find entry of given name and erase it
    return std::find_if(attached_entries.begin(), attached_entries.end(), is_same_name);
}

} /* namespace filesystem */
