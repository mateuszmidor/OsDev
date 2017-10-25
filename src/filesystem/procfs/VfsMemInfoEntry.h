/**
 *   @file: VfsMemInfoEntry.h
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSMEMINFOENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSMEMINFOENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

class VfsMemInfoEntry: public VfsEntry {
public:
    VfsMemInfoEntry() {}
    virtual ~VfsMemInfoEntry() {}

    // [common interface]
    bool is_directory() const override { return false; }
    const kstd::string& get_name() const { return name; }

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override { return false; }
    bool truncate(u32 new_size) override { return false; }

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override { return VfsEnumerateResult::ENUMERATION_FAILED; }

private:
    kstd::string get_info() const;
    const kstd::string  name            = "meminfo";
    bool                reading_done    = false;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSMEMINFOENTRY_H_ */
