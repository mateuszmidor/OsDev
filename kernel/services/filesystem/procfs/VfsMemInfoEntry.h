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

/**
 * @brief   This class exposes system memory information as virtual filesystem entry
 */
class VfsMemInfoEntry: public VfsEntry {
public:
    // [common interface]
    const cstd::string& get_name() const override                           { return name; }
    VfsEntryType get_type() const override                                  { return VfsEntryType::FILE; }
    utils::SyscallResult<void> open() override;
    utils::SyscallResult<void> close() override;

    // [file interface]
    utils::SyscallResult<u64> get_size() const override                     { return {0}; }
    utils::SyscallResult<u64> read(void* data, u32 count) override;
    utils::SyscallResult<u64> write(const void* data, u32 count) override   { return middlespace::ErrorCode::EC_PERM; }
    utils::SyscallResult<void> seek(u32 new_position) override              { return {INVALID_OP}; }
    utils::SyscallResult<void> truncate(u32 new_size) override              { return {INVALID_OP}; }
    utils::SyscallResult<u64> get_position() const override                 { return {0}; }

private:
    cstd::string get_info() const;
    bool is_open                {false};
    const cstd::string  name    {"meminfo"};
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSMEMINFOENTRY_H_ */
