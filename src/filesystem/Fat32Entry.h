/**
 *   @file: Fat32Entry.h
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32ENTRY_H_
#define SRC_FILESYSTEM_FAT32ENTRY_H_

#include "kstd.h"
#include "KernelLog.h"
#include "Fat32Structs.h"
#include "Fat32Table.h"
#include "Fat32Data.h"

namespace filesystem {

/**
 * @name    Fat32Entry
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
class Fat32Entry {
public:
    static Fat32Entry make_simple_dentry(const Fat32Table& fat_table, const Fat32Data& fat_data, const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index);
    static DirectoryEntryFat32 make_directory_entry_fat32(const Fat32Entry& e);
    Fat32Entry(const Fat32Entry& other) = default;
    Fat32Entry& operator=(const Fat32Entry& other);
    u32 read(void* data, u32 count);
//    u32 write(const void* data, u32 count);
    void seek(u32 new_position);
    operator bool() const;
    bool operator!() const;

    // useful data
    kstd::string    name;
    u32             size;
    bool            is_directory;
    u32             data_cluster;           // entry data cluster
    u32             position;               // read/write byte position
    u32             position_data_cluster;  // cluster where the "position" byte resides; keep it to reduce FatTable traversal and speedup cluster lookup

    // entry localization in parent dir, for file/dir operations
    u32             entry_cluster;
    u16             entry_sector;
    u8              entry_index;

    const Fat32Table&     fat_table;
    const Fat32Data&      fat_data;

private:
    friend class VolumeFat32;
    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data);
    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data, const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 entry_cluster, u16 entry_sector, u8 entry_index_no);

    utils::KernelLog& klog;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32ENTRY_H_ */
