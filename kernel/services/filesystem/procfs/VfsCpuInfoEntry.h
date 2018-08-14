/**
 *   @file: VfsCpuInfoEntry.h
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSCPUINFOENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSCPUINFOENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class exposes cpu information as virtual filesystem entry
 */
class VfsCpuInfoEntry: public VfsEntry {
public:
    // [common interface]
    const cstd::string& get_name() const override                           { return name; }
    VfsEntryType get_type() const override                                  { return VfsEntryType::FILE; }
    utils::SyscallResult<EntryState*> open() override;
    utils::SyscallResult<void> close(EntryState*) override;

    // [file interface]
    utils::SyscallResult<u64> get_size() const override                                         { return {0}; }
    utils::SyscallResult<u64> read(EntryState* state, void* data, u32 count) override;
    utils::SyscallResult<u64> write(EntryState* state, const void* data, u32 count) override    { return middlespace::ErrorCode::EC_PERM; }
    utils::SyscallResult<void> seek(EntryState* state, u32 new_position) override               { return {INVALID_OP}; }
    utils::SyscallResult<void> truncate(EntryState* state, u32 new_size) override               { return {INVALID_OP}; }
    utils::SyscallResult<u64> get_position(EntryState* state) const override                    { return {0}; }

private:
    const cstd::string  name                {"cpuinfo"};
    bool                is_open             {false};

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSCPUINFOENTRY_H_ */
