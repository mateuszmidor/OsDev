/**
 *   @file: VfsPseudoEntry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSRAMENTRY_H_
#define SRC_FILESYSTEM_VFSRAMENTRY_H_

#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory entry (file or directory)
 */
class VfsRamEntry: public VfsEntry {
public:
    VfsRamEntry(kstd::string name, bool is_dir = false);
    virtual ~VfsRamEntry();

    // [common interface]
    bool is_directory() const override;
    const kstd::string& get_name() const;

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override;
    bool truncate(u32 new_size) override;

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;


    kstd::string                name;
    u32                         size        = 0;
    bool                        is_dir      = false;
    kstd::vector<VfsEntryPtr>   entries;    // child entries
    static const u32            BUFF_SIZE   = 512;
    u8                          buff[BUFF_SIZE];  // in-memory file buffer
};

using VfsRamEntryPtr = std::shared_ptr<VfsRamEntry>;

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSRAMENTRY_H_ */
