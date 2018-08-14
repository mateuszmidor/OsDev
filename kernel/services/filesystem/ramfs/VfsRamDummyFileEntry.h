/**
 *   @file: VfsRamDummyFileEntry.h
 *
 *   @date: Aug 2, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This dummy file implementation is used for testing the VFS
 */
class VfsRamDummyFileEntry: public VfsEntry {
public:
    VfsRamDummyFileEntry(const cstd::string name) : name(name) {}

    // [common interface]
    const cstd::string& get_name() const override                                               { return name;                  }
    utils::SyscallResult<void> set_name(const cstd::string& name) override;
    VfsEntryType get_type() const override                                                      { return VfsEntryType::FILE;    }

    // [file interface]
    utils::SyscallResult<u64> read(EntryState* state, void* data, u32 count) override           { return {0};   }
    utils::SyscallResult<u64> write(EntryState* state, const void* data, u32 count) override    { return {0};   }

private:
    cstd::string name;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_ */
