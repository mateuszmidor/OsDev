/**
 *   @file: VfsFat32Entry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32ENTRY_H_
#define SRC_FILESYSTEM_VFSFAT32ENTRY_H_

#include "Fat32Entry.h"
#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class is Fat32Entry implementation for Virtual File System entry
 */
class VfsFat32Entry: public VfsEntry {
public:
    VfsFat32Entry(const Fat32Entry& e);
    ~VfsFat32Entry() override;

    // [common interface]
    bool is_directory() const override;
    const kstd::string& get_name() const;

    // [file interface]
    u32 get_size() const override;
    u32 read(void* data, u32 count) override;
    u32 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override;
    bool truncate(u32 new_size) override;

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

private:
    Fat32Entry      entry;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32ENTRY_H_ */
