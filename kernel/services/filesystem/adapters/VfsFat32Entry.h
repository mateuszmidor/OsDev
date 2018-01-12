/**
 *   @file: VfsFat32Entry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32ENTRY_H_
#define SRC_FILESYSTEM_VFSFAT32ENTRY_H_

#include "VfsEntry.h"
#include "fat32/Fat32Entry.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry adapter for Fat32Entry, so Fat32Entry can be used in VFS
 */
class VfsFat32Entry: public VfsEntry {
public:
    VfsFat32Entry(const Fat32Entry& e);
    ~VfsFat32Entry() override;

    // [common interface]
    bool open() override    { return true; /* no initialization to do here */ }
    void close() override   {};
    bool is_directory() const override;
    const kstd::string& get_name() const;

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override;
    bool truncate(u32 new_size) override;
    u32 get_position() const override;

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

private:
    Fat32Entry      entry;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32ENTRY_H_ */
