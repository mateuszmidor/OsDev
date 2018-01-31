/**
 *   @file: VfsDirectoryEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSDIRECTORYENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSDIRECTORYENTRY_H_

#include <errno.h>
#include "Vector.h"
#include "VfsEntry.h"

namespace filesystem {

class VfsDirectoryEntry: public VfsEntry {
public:
    // [common interface]
    bool open() override                                    { return true; }
    void close() override                                   { }
    bool is_directory() const override                      { return true; }
    bool is_mount_point() const override                    { return false; }

    // [file interface - not applicable for directory]
    u32 get_size() const override                           { return 0; };
    s64 read(void* data, u32 count) override                { return -EISDIR; };
    s64 write(const void* data, u32 count) override         { return -EISDIR; };
    bool seek(u32 new_position) override                    { return false; };
    bool truncate(u32 new_size) override                    { return false; };
    u32 get_position() const override                       { return 0; }

    // [directory interface]
    void attach_entry(const VfsEntryPtr entry) override  { nonpersistent_entries.push_back(entry); }
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

private:
    cstd::vector<VfsEntryPtr>   nonpersistent_entries;    // child entries that are not memory-stored, eg PIPE, SOCKET

};

using VfsDirectoryEntryPtr = std::shared_ptr<VfsDirectoryEntry>;

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSDIRECTORYENTRY_H_ */
