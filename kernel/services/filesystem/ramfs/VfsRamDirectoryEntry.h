/**
 *   @file: VfsRamDirectoryEntry.h
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_
#define KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_

#include "Vector.h"
#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory directory.
 */
class VfsRamDirectoryEntry: public VfsEntry {
public:
    VfsRamDirectoryEntry(const cstd::string& name) : name(name) {}

    // [common interface]
    const cstd::string& get_name() const override                                   { return name; };
    utils::SyscallResult<void> set_name(const cstd::string& name) override;
    VfsEntryType get_type() const override                                          { return VfsEntryType::DIRECTORY; }

    // [directory interface]
    utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& name) override;
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) override;
    bool attach_entry(const VfsEntryPtr& entry);
    bool detach_entry(const cstd::string& name);
    bool is_empty() const;

private:
    cstd::vector<VfsEntryPtr>::iterator find_entry(const cstd::string& name);
    cstd::vector<VfsEntryPtr>  entries;

    cstd::string  name;
};

using VfsRamDirectoryEntryPtr = std::shared_ptr<VfsRamDirectoryEntry>;

} /* namespace filesystem */

#endif /* KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_ */
