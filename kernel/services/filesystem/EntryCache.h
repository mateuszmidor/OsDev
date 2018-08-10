/**
 *   @file: EntryCache.h
 *
 *   @date: Jul 26, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_
#define KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_

#include "Map.h"
#include "VfsCachedEntry.h"

namespace filesystem {

/**
 * @brief   This class holds a mapping of path-to-cached entry
 *          Entries are being held either:
 *          1. directly in cache map
 *          2. as attachments to these stored in cache
 */
class EntryCache {
public:
    VfsCachedEntryPtr allocate(const VfsEntryPtr& e, const UnixPath& path);
    bool deallocate(const VfsCachedEntryPtr& e);
    VfsCachedEntryPtr find(const UnixPath& path) const;

private:
    cstd::Map<UnixPath, VfsCachedEntryPtr>   path_to_entry;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ENTRYCACHE_H_ */
