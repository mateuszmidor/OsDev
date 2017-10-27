/**
 *   @file: VfsPsInfoEntry.h
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class exposes running processes information as virtual filesystem entry
 */
class VfsPsInfoEntry: public VfsEntry {
    // [common interface]
    bool open() override;
    void close() override;
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
    kstd::string get_date_time() const;
    const kstd::string  name            = "psinfo";
    bool                is_open         = false;

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_ */
