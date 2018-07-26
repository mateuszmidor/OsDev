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
 * @brief   Prepare the entry cache to work with dynamic memory once it is available in system
 */
void EntryCache::install(const VfsEntryPtr& root_dir) {
    const u32 CACHE_MAX_ENTRIES {128};
    cached_entries.resize(CACHE_MAX_ENTRIES);
    allocate_entry(root_dir, "/");
}

/**
 * @brief   Unchecked access method for cached entries
 */
VfsCachedEntryPtr& EntryCache::operator[](GlobalFileDescriptor fd) {
    return cached_entries[fd];
}

const VfsCachedEntryPtr& EntryCache::operator[](GlobalFileDescriptor fd) const {
    return cached_entries[fd];
}

/**
 * @brief   Get global file descriptor if file pointed by "path" is already in cache, empty otherwise
 */
Optional<GlobalFileDescriptor> EntryCache::get_fd_for_path(const UnixPath& path) const {
    const auto found = path_to_filedescriptor.find(path);
    if (found == path_to_filedescriptor.cend())
        return {};

    return {found->second};
}

/**
 * @brief   Put entry to cache and return its file descriptor or empty if cache is full
 */
Optional<GlobalFileDescriptor> EntryCache::allocate_entry(const VfsEntryPtr& e, const UnixPath& path) {
    if (auto fd = find_free_fd()) {
        cached_entries[fd.value] = std::make_shared<VfsCachedEntry>(e);
        path_to_filedescriptor[path] = fd.value;
        return {fd.value};
    }

    return {};
}

/**
 * @brief   Remove entry pointed by valid file descriptor from cache
 */
void EntryCache::deallocate_entry(GlobalFileDescriptor fd) {
    path_to_filedescriptor.erase(path_to_filedescriptor.find_by_val(fd));
    cached_entries[fd].reset();
}

/**
 * @brief   Check if given file descriptor is in cache
 */
bool EntryCache::is_in_cache(GlobalFileDescriptor fd) const {
    if (fd >= cached_entries.size())
        return false;

    if (!cached_entries[fd])
        return false;

    return true;
}

/**
 * @brief  Return unused file descriptor if available, empty otherwise
 */
Optional<GlobalFileDescriptor> EntryCache::find_free_fd() const {
    for (u32 i = 0; i < cached_entries.size(); i++)
        if (!cached_entries[i])
            return {i};

    return {};
}
} /* namespace filesystem */
