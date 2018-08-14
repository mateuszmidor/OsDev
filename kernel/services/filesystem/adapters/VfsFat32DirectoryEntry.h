/**
 *   @file: VfsFat32DirectoryEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_

#include "VfsEntry.h"
#include "fat32/Fat32Entry.h"

namespace filesystem {

/**
 * @brief   This class is a Virtual File System adapter for Fat32 directory entry
 */
class VfsFat32DirectoryEntry: public VfsEntry {
public:
    VfsFat32DirectoryEntry(const Fat32Entry& e) : entry(e) {}

    // [common interface]
    const cstd::string& get_name() const override   { return entry.get_name(); }
    VfsEntryType get_type() const override          { return VfsEntryType::DIRECTORY; }

    // [directory interface]
    utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& path) override;
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) override;

private:
    VfsEntryPtr wrap_entry(const Fat32Entry& e) const;
    Fat32Entry  entry;

};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_ */
