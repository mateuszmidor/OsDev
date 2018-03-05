/**
 *   @file: OpenEntryCache.h
 *
 *   @date: Jan 29, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSCACHE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSCACHE_H_

#include "Map.h"
#include "UnixPath.h"
#include "VfsCachedEntry.h"

namespace filesystem {

/**
 * @brief   This class represents a path-entry map that can be used to cache filesystem entries.
 *          As it wraps VfsEntry into VfsCachedEntry, it allows for attaching extra entries to regular VfsEntry.
 *          This is useful for attaching eg. FIFO(pipe) entry to a regular directory.
 */
class VfsCache {
public:
    bool entry_exists(const UnixPath& path) const;
    VfsCachedEntryPtr get_entry(const UnixPath& path) const;
    VfsCachedEntryPtr add_entry(const UnixPath& path, const VfsEntryPtr& entry);
    bool delete_entry(const UnixPath& path);
    bool move_entry(const UnixPath& path_from, const UnixPath& path_to);
    bool copy_entry(const UnixPath& path_from, const UnixPath& path_to);

private:
    VfsCachedEntryPtr find_cached_entry(const UnixPath& path) const;

    cstd::Map<UnixPath, VfsCachedEntryPtr>    entries;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSCACHE_H_ */
