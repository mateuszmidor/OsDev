/**
 *   @file: VfsFat32FileEntry.h
 *
 *   @date: Jan 30, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_

#include "VfsFileEntry.h"
#include "fat32/Fat32Entry.h"

namespace filesystem {

class VfsFat32FileEntry: public VfsFileEntry {
public:
    VfsFat32FileEntry(const Fat32Entry& e);

    // [common interface]
    const cstd::string& get_name() const;

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override;
    bool truncate(u32 new_size) override;
    u32 get_position() const override;

private:
    Fat32Entry      entry;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_ADAPTERS_VFSFAT32FILEENTRY_H_ */
