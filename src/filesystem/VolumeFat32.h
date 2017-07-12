/**
 *   @file: VolumeFat32.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 *    @see: https://www.pjrc.com/tech/8051/ide/fat32.html
 */

#ifndef SRC_FILESYSTEM_VOLUMEFAT32_H_
#define SRC_FILESYSTEM_VOLUMEFAT32_H_

#include "Fat32Table.h"
#include "Fat32Data.h"
#include "kstd.h"
#include "AtaDriver.h"

namespace filesystem {


/**
 * @name    VolumeFat32
 * @brief   Fat32 volume/partition abstraction.
 */
class VolumeFat32 {
public:
    VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors);
    kstd::string get_label() const;
    kstd::string get_type() const;
    u32 get_size_in_bytes() const;
    u32 get_used_space_in_bytes() const;
    u32 get_used_space_in_clusters() const;

    bool get_entry(const kstd::string& unix_path, SimpleDentryFat32& e) const;
    u32 read_file_entry(const SimpleDentryFat32& file, void* data, u32 count) const;
    EnumerateResult enumerate_directory_entry(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry_found) const;
    bool create_entry(const kstd::string& unix_path, bool directory) const;
    bool delete_entry(const kstd::string& unix_path) const;

private:
    SimpleDentryFat32 get_root_dentry() const;
    bool get_entry_for_name(const SimpleDentryFat32& dentry, const kstd::string& filename, SimpleDentryFat32& out) const;

    // delete_file stuff
    void remove_dir_cluster_if_empty(const SimpleDentryFat32& dentry, u32 cluster) const;
    bool alloc_entry_in_directory(const SimpleDentryFat32& dir, SimpleDentryFat32 &e) const;
    bool is_directory_empty(const SimpleDentryFat32& e) const;


    drivers::AtaDevice& hdd;
    VolumeBootRecordFat32 vbr;
    Fat32Table fat_table;
    Fat32Data fat_data;

    bool bootable;
    u32 partition_offset_in_sectors;
    u32 partition_size_in_sectors;

    u32 fat_start;
    u32 data_start;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VOLUMEFAT32_H_ */
