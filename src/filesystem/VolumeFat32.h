/**
 *   @file: VolumeFat32.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 *    @see: https://www.pjrc.com/tech/8051/ide/fat32.html
 */

#ifndef SRC_FILESYSTEM_VOLUMEFAT32_H_
#define SRC_FILESYSTEM_VOLUMEFAT32_H_

#include <functional>
#include "kstd.h"
#include "AtaDriver.h"
#include "ScreenPrinter.h"

namespace filesystem {

/**
 * @name    VolumeBootRecordFat32
 * @brief   Volume metainfo; located at partition first sector
 */
struct VolumeBootRecordFat32 {
    u8  jump[3];
    u8  software_name[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fat_table_copies;       // how many copies of FAT
    u16 root_dir_entries;       // obsolete
    u16 total_sectors;
    u8  media_type;             // hdd/floppy/...
    u16 fat_sector_count;       // obsolete
    u16 sectors_per_track;
    u16 head_count;
    u32 hidden_sectors;
    u32 total_sector_count;

    u32 fat_table_size_in_sectors;         // how many sectors makes a single FAT
    u16 ext_flags;
    u16 fat_version;
    u32 root_cluster;
    u16 fat_info;
    u16 backup_sector;
    u8  reserved0[12];
    u8  drive_number;
    u8  reserved;
    u8  boot_signature;
    u32 volume_id;
    u8  volume_label[11];
    u8  fat_type_label[8];
} __attribute__((packed));

/**
 * @name    DirectoryEntryFat32
 * @brief   Fat32 directory entry
 */
struct DirectoryEntryFat32 {
    u8  name[8];
    u8  ext[3];
    u8  attributes;         // see: DirectoryEntryFat32Attrib
    u8  reserved;
    u8  c_time_tenth;       // creation time
    u16 c_time;
    u16 c_date;
    u16 a_time;             // access time

    u16 first_cluster_hi;

    u16 w_time;             // write time
    u16 w_date;
    u16 first_cluster_lo;
    u32 size;
} __attribute__((packed));

enum DirectoryEntryFat32Attrib : u8 {
    READ_ONLY = 0x01, // Should not allow writing
    HIDDEN    = 0x02, // Should not show in dir listing
    SYSTEM    = 0x04, // Should not be moved physically; OS file
    VOLUMEID  = 0x08, // Filename is Volume ID
    LONGNAME  = 0x0F, // Entry providing long filename
    DIRECTORY = 0x10, // Is a subdirectory (16 x 32-byte records)
    ARCHIVE   = 0x20  // Has been changed since last backup
};

/**
 * @name    SimpleDentryFat32
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
struct SimpleDentryFat32 {
    SimpleDentryFat32() : SimpleDentryFat32("", 0, false, 0) {}
    SimpleDentryFat32(const kstd::string& name, u32 size, bool is_directory, u32 cluster_no):
        name(name), size(size), is_directory(is_directory), cluster_no(cluster_no) {}

    kstd::string    name;
    u32             size;
    bool            is_directory;
    u32             cluster_no;
};

/**
 * @name    VolumeFat32
 * @brief   Fat32 volume/partition abstraction.
 */
class VolumeFat32 {
public:
    // return true to continue directory contents iteration
    using OnEntryFound = std::function<bool(const SimpleDentryFat32& e)>;

    VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors);

    kstd::string get_label() const;
    kstd::string get_type() const;
    u32 get_size_in_bytes() const;
    u32 get_used_space_in_bytes() const;

    bool get_entry_for_path(const kstd::string& unix_path, SimpleDentryFat32& e) const;
    bool enumerate_directory(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry_found) const;
    u32 read_file(const SimpleDentryFat32& file, void* data, u32 count) const;

private:
    SimpleDentryFat32 get_root_dentry() const;
    bool get_entry_for_name(const SimpleDentryFat32& dentry, const kstd::string& filename, SimpleDentryFat32& out) const;
    u32 get_next_cluster(u32 current_cluster) const;
    bool read_fat_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const;
    bool read_fat_table_sector(u32 sector, void* data, u32 size) const;
    SimpleDentryFat32 make_simple_dentry(const DirectoryEntryFat32& dentry) const;


    static const u8 DIR_ENTRY_NO_MORE = 0x00;   // First byte of dir entry == 0 means there is no more entries in this dir
    static const u8 DIR_ENTRY_UNUSED  = 0xE5;   // Unused entry means the file was deleted

    static const u32 CLUSTER_UNUSED             = 0;            // In Fat32 table, unused clusters are marked as 0
    static const u32 CLUSTER_FIRST_VALID        = 2;            // Clusters 0 and 1 are reserved, 2 usually is the cluster of root dir
    static const u32 CLUSTER_END_OF_FILE        = 0x0FFFFFF8;   // Such entry in Fat32 table indicates we've reached last cluster in file chain
    static const u32 CLUSTER_END_OF_DIRECTORY   = 0x0FFFFFFF;   // Such entry in Fat32 table indicates we've reached the last cluster in dir chain
    static const u32 FAT32_CLUSTER_28BIT_MASK   = 0x0FFFFFFF;   // Fat32 table cluster index actually use 28 bits, highest 4 bits should be ignored

    drivers::AtaDevice& hdd;
    VolumeBootRecordFat32 vbr;
    bool bootable;
    u32 partition_offset_in_sectors;
    u32 partition_size_in_sectors;

    u32 fat_start;
    u32 data_start;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VOLUMEFAT32_H_ */
