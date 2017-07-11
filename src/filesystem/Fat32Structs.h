/**
 *   @file: Fat32Structs.h
 *
 *   @date: Jul 10, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32STRUCTS_H_
#define SRC_FILESYSTEM_FAT32STRUCTS_H_

#include "types.h"

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

} // filesystem

#endif /* SRC_FILESYSTEM_FAT32STRUCTS_H_ */
