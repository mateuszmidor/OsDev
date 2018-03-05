/**
 *   @file: VfsDateEntry.h
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_

#include "VfsEntry.h"
#include "Port.h"

namespace filesystem {

/**
 * @brief   This class exposes system date and time as virtual filesystem entry
 */
class VfsDateEntry: public VfsEntry {
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
    cstd::string get_date_time() const;
    u8 read_byte(u8 offset) const;
    u8 to_bin(u8 bcd) const;
    const cstd::string      name            {"date"};
    bool                    is_open         {false};
    hardware::Port8bitSlow  address         {0x70};
    hardware::Port8bitSlow  data            {0x71};
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_ */
