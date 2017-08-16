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


    static const u8 DIR_ENTRY_NO_MORE           = 0x00;   // entry.name[0] == 0x00 means there is no more entries in this dir
    static const u8 DIR_ENTRY_UNUSED            = 0xE5;   // entry.name[0] == 0xE5 means the file was deleted

    /**
     * @brief   Is this a meta entry describing volume id?
     */
    bool is_volume_id() const {
        return (attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID;
    }

    /**
     * @brief   Is this a meta entry describing long (more than 8.3) file name?
     */
    bool is_long_name() const {
        return (attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME;
    }

    /**
     * @brief   Is this a meta entry describing empty space (eg. space after deleted file/directory)?
     */
    bool is_unused() const {
        return (name[0] == DIR_ENTRY_UNUSED) ||
               (name[0] == '.' && name[1] == ' ') ||                    // treat dot folder as unused (why waste space for it?)
               (name[0] == '.' && name[1] == '.' && name[2] == ' ');    // treat dot-dot folder as unused (why waste space for it?)
    }

    /**
     * @brief   Is this a meta entry describing end of directory contents?
     */
    bool is_nomore() const {
        return name[0] == DIR_ENTRY_NO_MORE;
    }

} __attribute__((packed));  // 32 bytes in total


} // filesystem

#endif /* SRC_FILESYSTEM_FAT32STRUCTS_H_ */
