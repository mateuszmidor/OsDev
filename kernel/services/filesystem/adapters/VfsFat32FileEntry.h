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

/**
 * @brief   This class is a  Virtual File System adapter for Fat32 file entry
 */
class VfsFat32FileEntry: public VfsEntry {
public:
    VfsFat32FileEntry(const Fat32Entry& e);

    // [common interface]
    const cstd::string& get_name() const override                                               { return entry.get_name();                  }
    VfsEntryType get_type() const override                                                      { return VfsEntryType::FILE;                }
    utils::SyscallResult<EntryState*> open() override                                           { return new Fat32State(entry.open());      }
    utils::SyscallResult<void> close(EntryState* state) override                                { delete state; return {middlespace::ErrorCode::EC_OK};     }

    // [file interface]
    utils::SyscallResult<u64> get_size() const override                                         { return {entry.get_size()};                                }
    utils::SyscallResult<u64> read(EntryState* state, void* data, u32 count) override           { return {entry.read(*(Fat32State*)state, data, count)};     }
    utils::SyscallResult<u64> write(EntryState* state, const void* data, u32 count) override    { return {entry.write(*(Fat32State*)state, data, count)};    }
    utils::SyscallResult<void> seek(EntryState* state, u32 new_position) override;
    utils::SyscallResult<void> truncate(EntryState* state, u32 new_size) override;
    utils::SyscallResult<u64> get_position(EntryState* state) const override                    { return {entry.get_position(*(Fat32State*)state)};          }

private:
    Fat32Entry      entry;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_ */
