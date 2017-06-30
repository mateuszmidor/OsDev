/**
 *   @file: Fat32.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_H_
#define SRC_FILESYSTEM_FAT32_H_

#include "kstd.h"
#include "AtaDriver.h"
#include "ScreenPrinter.h"

namespace filesystem {

struct BiosParameterBlock32 {
    u8  jump[3];
    u8  software_name[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fat_copies;         // how many copies of FAT
    u16 root_dir_entries;   // obsolete
    u16 total_sectors;
    u8  media_type;         // hdd/floppy/...
    u16 fat_sector_count;   // obsolete
    u16 sectors_per_track;
    u16 head_count;
    u32 hidden_sectors;
    u32 total_sector_count;

    u32 table_size;         // how many sectors makes a single FAT
    u16 ext_flags;
    u16 fat_version;
    u32 root_cluster_count;
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

struct DirectoryEntry32 {
    u8  name[8];
    u8  ext[3];
    u8  attributes;
    u8  reserved;
    u8  c_time_tenth;   // creation
    u16 c_time;
    u16 c_date;
    u16 a_time;         // access

    u16 first_cluster_hi;

    u16 w_time;         // write
    u16 w_date;
    u16 first_cluster_lo;
    u32 size;
} __attribute__((packed));

class Fat32 {
public:
    Fat32(drivers::AtaDevice& hdd);
    BiosParameterBlock32 read_bios_block(u32 partition_offset);
    void print_bios_block(const BiosParameterBlock32& bpb, ScreenPrinter& printer);
    kstd::vector<DirectoryEntry32> read_root_directory(u32 partition_offset);
    void print_directory_entries(const BiosParameterBlock32& bpb, u32 partition_offset, const kstd::vector<DirectoryEntry32>& entries, ScreenPrinter& printer);

    kstd::string trim(const u8* in, u16 len);

private:
    drivers::AtaDevice& hdd;
    static const int ATTR_ILLEGAL = 0x0F;
    static const int ATTR_DIRECTORY = 0x10;
};

} /* namespace cpuexceptions */

#endif /* SRC_FILESYSTEM_FAT32_H_ */
