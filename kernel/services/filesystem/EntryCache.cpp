/**
 *   @file: EntryCache.cpp
 *
 *   @date: Jul 26, 2018
 * @author: Mateusz Midor
 */

#include "EntryCache.h"

using namespace cstd;
using namespace middlespace;

namespace filesystem {

/**
 * @brief   Put entry to cache and return its cached version
 */
VfsCachedEntryPtr EntryCache::allocate(const VfsEntryPtr& e, const UnixPath& path) {
    auto cached = std::make_shared<VfsCachedEntry>(e);
    path_to_entry[path] = cached;
    return cached;
}

/**
 * @brief   Remove cached entry from cache, return true on success, false otherwise
 */
bool EntryCache::deallocate(const VfsCachedEntryPtr& e) {
    const auto found = path_to_entry.find_by_val(e);
    if (found == path_to_entry.cend())
        return false;

    path_to_entry.erase(found);
    return true;
}

/**
 * @brief   Get cached entry if entry pointed by "path" is already in cache, empty otherwise
 */
VfsCachedEntryPtr EntryCache::find(const UnixPath& path) const {
    const auto found = path_to_entry.find(path);
    if (found == path_to_entry.cend())
        return {};

    return {found->second};
}
} /* namespace filesystem */
