/**
 *   @file: OpenEntryCache.h
 *
 *   @date: Jan 29, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSRAMSTORAGE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSRAMSTORAGE_H_

#include "Map.h"
#include "VfsEntry.h"
#include "UnixPath.h"

namespace filesystem {

class VfsRamStorage {
public:
    VfsEntryPtr get_entry(const UnixPath& path);
    void add_entry(const UnixPath& path, VfsEntryPtr entry);
    bool delete_entry(const UnixPath& path);
    bool move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to);
    bool copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to);

private:
    cstd::Map<UnixPath, std::weak_ptr<VfsEntry>>    entries;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSRAMSTORAGE_H_ */
