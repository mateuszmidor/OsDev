/**
 *   @file: OpenEntryCache.cpp
 *
 *   @date: Jan 29, 2018
 * @author: Mateusz Midor
 */

#include "VfsCache.h"

namespace filesystem {

/**
 * @brief   Check if entry under "path" exists, either directly stored or as an attachment
 */
bool VfsCache::entry_exists(const UnixPath& path) const {
    return (bool)get_entry(path);
}

/**
 * @brief   Get entry at given "path", either directly stored or attached to another entry
 */
VfsCachedEntryPtr VfsCache::get_entry(const UnixPath& path) const {
    // get directly stored entry
    if (auto result = find_cached_entry(path))
        return result;

    // get entry stored as parent-attachment
    if (auto parent = find_cached_entry(path.extract_directory()))
         return parent->get_attached_entry(path.extract_file_name());

    return {};
}

/**
 * @brief   Find entry at given "path", only consider directly stored
 */
VfsCachedEntryPtr VfsCache::find_cached_entry(const UnixPath& path) const {
    const auto it = entries.find(path);
    if (it != entries.cend())
        return it->second;

    return {};
}

/**
 * @brief   Add an "entry" under given "path"
 */
VfsCachedEntryPtr VfsCache::add_entry(const UnixPath& path, const VfsEntryPtr& entry) {
    // check entry already exists
    if (entry_exists(path))
        return {};

    VfsCachedEntryPtr d {std::make_shared<VfsCachedEntry>(entry)};
    entries[path] = d;
    return d;
}

/**
 * @brief   Delete entry under given "path"
 */
bool VfsCache::delete_entry(const UnixPath& path) {
    bool anything_deleted {false};

    // remove directly stored entry
    auto it = entries.find(path);
    if (it != entries.end()) {
         entries.erase(it);
         anything_deleted |= true;
    }

    // remove entry stored as parent-attachment
    if (auto parent = find_cached_entry(path.extract_directory()))
        anything_deleted |= parent->detach_entry(path.extract_file_name());

    return anything_deleted;
}

/**
 * @brief   Move entry to another location
 */
bool VfsCache::move_entry(const UnixPath& path_from, const UnixPath& path_to) {
    if (!copy_entry(path_from, path_to))
        return false;

    if (!delete_entry(path_from))
        return false;

    return true;
}

/**
 * @brief   Copy entry to another location
 */
bool VfsCache::copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    if (auto e = get_entry(unix_path_from))
        return (bool)add_entry(unix_path_to, e);

    // unix_path_from doesnt exist
    return false;
}

} /* namespace filesystem */
