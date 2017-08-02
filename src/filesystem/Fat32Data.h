/**
 *   @file: Fat32Data.h
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32DATA_H_
#define SRC_FILESYSTEM_FAT32DATA_H_

#include <functional>
#include "kstd.h"
#include "AtaDriver.h"
#include "Fat32Structs.h"

namespace filesystem {

/**
 * @name    SimpleDentryFat32
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
struct SimpleDentryFat32 {
    SimpleDentryFat32() : SimpleDentryFat32("", 0, false, 0, 0, 0, 0) {}
    SimpleDentryFat32(const kstd::string& name,
            u32 size,
            bool is_directory,
            u32 data_cluster,
            u32 entry_cluster,
            u16 entry_sector,
            u8 entry_index_no):
        name(name),
        size(size),
        is_directory(is_directory),
        data_cluster(data_cluster),
        entry_cluster(entry_cluster),
        entry_sector(entry_sector),
        entry_index(entry_index_no),
        position(0),
        position_data_cluster(data_cluster){}


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

// directory enumeration result
enum class EnumerateResult { ENUMERATION_FINISHED, ENUMERATION_STOPPED, ENUMERATION_CONTINUE };

// action to take on entry enumeration. Return true to continue directory contents enumeration
using OnEntryFound = std::function<bool(SimpleDentryFat32& e)>;


class Fat32Data {
public:
    Fat32Data(drivers::AtaDevice& hdd);
    void setup(u32 data_start, u16 bytes_per_sector, u8 sectors_per_cluster);

    bool read_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const;
    bool read_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void* data, u32 size) const;
    bool write_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const;
    bool write_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void const* data, u32 size) const;
    void clear_data_cluster(u32 cluster) const;
    EnumerateResult enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector = 0, u8 start_index = 0) const;
    u8 get_free_entry_in_dir_cluster(u32 cluster, SimpleDentryFat32& out) const;
    void write_entry(const SimpleDentryFat32& e) const;
    void mark_entry_as_nomore(const SimpleDentryFat32& e) const;
    void mark_next_entry_as_nomore(const SimpleDentryFat32& e) const;
    void mark_entry_as_unused(const SimpleDentryFat32& e) const;
    void set_entry_data_cluster(const SimpleDentryFat32& e, u32 first_cluster) const;
    bool is_directory_cluster_empty(u32 cluster) const;
    SimpleDentryFat32 make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) const;
    DirectoryEntryFat32 make_directory_entry_fat32(const SimpleDentryFat32& e) const;

    static const u8 DIR_ENTRY_NO_MORE   = 0x00;   // First byte of dir entry == 0 means there is no more entries in this dir
    static const u8 DIR_ENTRY_UNUSED    = 0xE5;   // Unused entry means the file was deleted
    static const u8 DIR_ENTRY_NOT_FOUND = 0xFF;   // No dir entry found

private:
    drivers::AtaDevice& hdd;
    u32 data_start_in_sectors   = 0;
    u16 bytes_per_sector        = 0;
    u8 sectors_per_cluster      = 0;
    static constexpr u8 FAT32ENTRIES_PER_SECTOR = 16; // BYTES_PER_SECTOR / sizeof(DirectoryEntryFat32);
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32DATA_H_ */
