/**
 *   @file: Fat32Entry.h
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32ENTRY_H_
#define SRC_FILESYSTEM_FAT32ENTRY_H_

#include <functional>
#include "kstd.h"
#include "KernelLog.h"
#include "Fat32Structs.h"
#include "Fat32Table.h"
#include "Fat32Data.h"

namespace filesystem {

// directory enumeration result
enum class EnumerateResult { ENUMERATION_FINISHED, ENUMERATION_STOPPED, ENUMERATION_CONTINUE };

// action to take on entry enumeration. Return true to continue directory contents enumeration
class Fat32Entry;
using OnEntryFound = std::function<bool(Fat32Entry& e)>;


/**
 * @name    Fat32Entry
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
class Fat32Entry {
public:
    static DirectoryEntryFat32 make_directory_entry_fat32(const Fat32Entry& e);

    Fat32Entry(const Fat32Entry& other) = default;
    Fat32Entry& operator=(const Fat32Entry& other);

    u32 read(void* data, u32 count);
    u32 write(const void* data, u32 count);
    void seek(u32 new_position);
    void truncate(u32 new_size);
    EnumerateResult enumerate_entries(const OnEntryFound& on_entry) const;

    operator bool() const;
    bool operator!() const;

    // useful data
    kstd::string    name;
    u32             size;
    bool            is_directory;
    u32             data_cluster;           // entry data cluster
    u32             current_position;       // read/write byte position
    u32             current_data_cluster;   // cluster where the "current_position" byte resides; keep it to reduce FatTable traversal and speedup cluster lookup

    // entry localization in parent dir, for file/dir operations
    u32             entry_cluster;
    u16             entry_sector;
    u8              entry_index;

    const Fat32Table&   fat_table;
    const Fat32Data&    fat_data;

private:
    friend class VolumeFat32;

    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data);
    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data, const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 entry_cluster, u16 entry_sector, u8 entry_index_no);
    void write_entry() const;
    u32 get_cluster_for_write();
    u32 attach_next_cluster(u32 cluster) const;
    EnumerateResult enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector = 0, u8 start_index = 0) const;
    Fat32Entry make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) const;

    utils::KernelLog&   klog;

    static constexpr u8 FAT32ENTRIES_PER_SECTOR = 16; // BYTES_PER_SECTOR / sizeof(DirectoryEntryFat32);
    static const u8 DIR_ENTRY_NO_MORE           = 0x00;   // First byte of dir entry == 0 means there is no more entries in this dir
    static const u8 DIR_ENTRY_UNUSED            = 0xE5;   // Unused entry means the file was deleted
    static const u8 DIR_ENTRY_NOT_FOUND         = 0xFF;   // No dir entry found
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32ENTRY_H_ */
