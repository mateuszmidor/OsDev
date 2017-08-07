/**
 *   @file: Fat32Entry.h
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32ENTRY_H_
#define SRC_FILESYSTEM_FAT32ENTRY_H_

#include "types.h"
#include "kstd.h"

namespace filesystem {

/**
 * @name    Fat32Entry
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
class Fat32Entry {
public:
    Fat32Entry();
    Fat32Entry(const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 entry_cluster,  u16 entry_sector, u8 entry_index_no);

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
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32ENTRY_H_ */
