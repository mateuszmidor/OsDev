/**
 *   @file: VfsFat32FileEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_

#include "VfsEntry.h"
#include "fat32/Fat32Entry.h"

namespace filesystem {

class VfsFat32FileEntry: public VfsEntry {
public:
    VfsFat32FileEntry(const Fat32Entry& e);

    // [common interface]
    const cstd::string& get_name() const override                           { return entry.get_name();                  }
    VfsEntryType get_type() const override                                  { return VfsEntryType::FILE;                }
    utils::SyscallResult<void> open() override                              { return {middlespace::ErrorCode::EC_OK};   }
    utils::SyscallResult<void> close() override                             { return {middlespace::ErrorCode::EC_OK};   }

    // [file interface]
    utils::SyscallResult<u64> get_size() const override                     { return {entry.get_size()};                }
    utils::SyscallResult<u64> read(void* data, u32 count) override          { return {entry.read(state, data, count)};  }
    utils::SyscallResult<u64> write(const void* data, u32 count) override   { return {entry.write(state, data, count)}; }
    utils::SyscallResult<void> seek(u32 new_position) override;
    utils::SyscallResult<void> truncate(u32 new_size) override;
    utils::SyscallResult<u64> get_position() const override                 { return {entry.get_position(state)};       }

private:
    Fat32Entry      entry;
    Fat32State      state;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_ */
