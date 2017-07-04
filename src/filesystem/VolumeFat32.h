/**
 *   @file: VolumeFat32.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 *    @see: https://www.pjrc.com/tech/8051/ide/fat32.html
 */

#ifndef SRC_FILESYSTEM_VOLUMEFAT32_H_
#define SRC_FILESYSTEM_VOLUMEFAT32_H_

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

    u32 fat_table_size;         // how many sectors makes a single FAT
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
    u8  attributes;
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

/**
 * @name    VolumeFat32
 * @brief   Fat32 volume/partition abstraction.
 */
class VolumeFat32 {
public:
    VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors);
    void print_volume_info(ScreenPrinter& printer) const;
    void print_root_tree(ScreenPrinter& printer) const;

private:
    void print_dir_tree(u32 dentry_cluster, ScreenPrinter& printer, u8 level) const;
    void print_file(u32 file_cluster, u32 file_size, ScreenPrinter& printer) const;
    u32 get_next_cluster(u32 current_cluster) const;
    bool read_fat_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const;
    bool read_fat_table_sector(u32 sector, void* data, u32 size) const;

    static const int ATTR_READ_ONLY = 0x01; // Should not allow writing
    static const int ATTR_HIDDEN    = 0x02; // Should not show in dir listing
    static const int ATTR_SYSTEM    = 0x04; // Should not be moved physically; OS file
    static const int ATTR_VOLUMEID  = 0x08; // Filename is Volume ID
    static const int ATTR_DIRECTORY = 0x10; // Is a subdirectory (32-byte records)
    static const int ATTR_ARCHIVE   = 0x20; // Has been changed since last backup
    static const int ATTR_LONGNAME  = 0x0F; // Entry providing long filename

    static const u8 DIR_ENTRY_NO_MORE = 0x00;   // First byte of dir entry == 0 means there is no more entries in this dir
    static const u8 DIR_ENTRY_UNUSED  = 0xE5;   // Unused entry means the file was deleted

    static const u32 CLUSTER_END_OF_FILE        = 0x0FFFFFF8;
    static const u32 CLUSTER_END_OF_CHAIN       = 0x0FFFFFFF;     // Such entry in Fat32 table indicates we've reached the last cluster in the chain
    static const u32 FAT32_CLUSTER_28BIT_MASK   = 0x0FFFFFFF; // Fat32 table cluster index actually use 28 bits, highest 4 bits should be ignored

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
