/**
 *   @file: VfsFat32DirectoryEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_

#include "VfsDirectoryEntry.h"
#include "fat32/Fat32Entry.h"

namespace filesystem {

class VfsFat32DirectoryEntry: public VfsDirectoryEntry {
public:
    VfsFat32DirectoryEntry(const Fat32Entry& e) : entry(e) {}

    // [common interface]
    const cstd::string& get_name() const override   { return entry.get_name(); }

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

private:
    VfsEntryPtr wrap_entry(const Fat32Entry& e) const;
    Fat32Entry      entry;

};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32DIRECTORYENTRY_H_ */
