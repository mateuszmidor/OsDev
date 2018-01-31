/**
 *   @file: VfsPsInfoEntry.h
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_

#include "VfsFileEntry.h"

namespace filesystem {

/**
 * @brief   This class exposes running processes information as virtual filesystem entry
 */
class VfsPsInfoEntry: public VfsFileEntry {
public:
    // [common interface]
    bool open() override;
    void close() override;
    const cstd::string& get_name() const    { return name; }

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override    { return false; }
    bool truncate(u32 new_size) override    { return false; }
    u32 get_position() const override       { return 0; }

private:
    const cstd::string  name                {"psinfo"};
    bool                is_open             {false};

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSPSINFOENTRY_H_ */
