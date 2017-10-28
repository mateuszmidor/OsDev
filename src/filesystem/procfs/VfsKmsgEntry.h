/**
 *   @file: kmsg.h
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSKMSGENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSKMSGENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class exposes kernel log as virtual filesystem entry
 */
class VfsKmsgEntry: public VfsEntry {
public:
    VfsKmsgEntry() {}
    virtual ~VfsKmsgEntry() {}

    // [common interface]
    bool open() override                    { return true; /* no initialization to do here */ }
    void close() override                   {};
    bool is_directory() const override      { return false; }
    const kstd::string& get_name() const    { return name; }

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override    { return false; }
    bool truncate(u32 new_size) override    { return false; }
    u32 get_position() const override       { return 0; }

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override { return VfsEnumerateResult::ENUMERATION_FAILED; }

private:
    const kstd::string    name = "kmsg";
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSKMSGENTRY_H_ */
