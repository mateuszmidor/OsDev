/**
 *   @file: EntryCache.h
 *
 *   @date: Jul 26, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_
#define KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_

#include "Map.h"
#include "Optional.h"
#include "VfsCachedEntry.h"

namespace filesystem {

/**
 * @brief   Represents a file descriptor that is global throughout the system
 */
using GlobalFileDescriptor = u32;

/**
 * @brief   This class holds a vector of cached vfs entries indexed by its file descriptors
 *          and a mapping of path-to-filedescriptor
 *          Entries are being held either:
 *          1. directly in cache
 *          2. as attachments to these stored in cache (they dont have their filedescriptors and are not considered as cached)
 */
class EntryCache {
public:
    void install();
    VfsCachedEntryPtr& operator[](GlobalFileDescriptor fd);
    const VfsCachedEntryPtr& operator[](GlobalFileDescriptor fd) const;
    cstd::Optional<GlobalFileDescriptor> find(const UnixPath& path) const;
    cstd::Optional<GlobalFileDescriptor> allocate(const VfsEntryPtr& e, const UnixPath& path);
    void deallocate(GlobalFileDescriptor fd);
    bool is_in_cache(GlobalFileDescriptor fd) const;

private:
    cstd::Optional<GlobalFileDescriptor> find_free_fd() const;
    cstd::Map<UnixPath, GlobalFileDescriptor>   path_to_filedescriptor;
    cstd::vector<VfsCachedEntryPtr>             cached_entries;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_ */
