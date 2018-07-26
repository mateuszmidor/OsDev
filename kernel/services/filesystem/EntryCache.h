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
 */
class EntryCache {
public:
    void install(const VfsEntryPtr& root_dir);
    VfsCachedEntryPtr& operator[](GlobalFileDescriptor fd);
    const VfsCachedEntryPtr& operator[](GlobalFileDescriptor fd) const;
    cstd::Optional<GlobalFileDescriptor> get_fd_for_path(const UnixPath& path) const;
    cstd::Optional<GlobalFileDescriptor> allocate_entry(const VfsEntryPtr& e, const UnixPath& path);
    void deallocate_entry(GlobalFileDescriptor fd);
    bool is_in_cache(GlobalFileDescriptor fd) const;

private:
    cstd::Optional<GlobalFileDescriptor> find_free_fd() const;
    cstd::Map<UnixPath, GlobalFileDescriptor>   path_to_filedescriptor;
    cstd::vector<VfsCachedEntryPtr>             cached_entries;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_ */
