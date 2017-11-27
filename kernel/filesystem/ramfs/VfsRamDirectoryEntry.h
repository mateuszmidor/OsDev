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
#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory directory.
 */
class VfsRamDirectoryEntry: public VfsEntry {
private:
    kstd::string                name;
    kstd::vector<VfsEntryPtr>   entries;    // child entries

public:
    VfsRamDirectoryEntry(const kstd::string& name) : name(name) {}

    // [common interface]
    bool open() override                            { return true; }
    void close() override                           {};
    bool is_directory() const override              { return true; }
    const kstd::string& get_name() const override   { return name; }

    // [file interface]
    u32 get_size() const override                       { return 0; };
    s64 read(void* data, u32 count) override            { return -EISDIR; };
    s64 write(const void* data, u32 count) override     { return -EISDIR; };
    bool seek(u32 new_position) override                { return false; };
    bool truncate(u32 new_size) override                { return false; };
    u32 get_position() const override                   { return 0; }

    // [directory interface]
    void push_back(const VfsEntryPtr entry)             { entries.push_back(entry); }
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;
};

using VfsRamDirectoryEntryPtr = std::shared_ptr<VfsRamDirectoryEntry>;

} /* namespace filesystem */

#endif /* KERNEL_FILESYSTEM_RAMFS_VFSRAMDIRECTORYENTRY_H_ */
