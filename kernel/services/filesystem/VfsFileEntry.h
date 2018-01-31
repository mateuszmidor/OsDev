/**
 *   @file: VfsFileEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSFILEENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSFILEENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

class VfsFileEntry: public VfsEntry {
public:
    // [common interface]
    bool open() override                    { return true; }
    void close() override                   { }
    bool is_directory() const override      { return false; }
    bool is_mount_point() const override    { return false; }

    // [file interface]
    // to be implemented by actual implementation

    // [directory interface - not applicable for file]
    void attach_entry(const VfsEntryPtr entry) override { }
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override { return VfsEnumerateResult::ENUMERATION_FAILED; } ;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSFILEENTRY_H_ */
