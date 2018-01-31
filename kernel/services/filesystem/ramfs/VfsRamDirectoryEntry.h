/**
 *   @file: VfsRamDirectoryEntry.h
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_
#define KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_

#include <errno.h>
#include "Vector.h"
#include "VfsDirectoryEntry.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory directory.
 */
class VfsRamDirectoryEntry: public VfsDirectoryEntry {
public:
    // [common interface]
    VfsRamDirectoryEntry(const cstd::string& name) : name(name) {}
    const cstd::string& get_name() const override   { return name; };

private:
    const cstd::string  name;
};

using VfsRamDirectoryEntryPtr = std::shared_ptr<VfsRamDirectoryEntry>;

} /* namespace filesystem */

#endif /* KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_ */
