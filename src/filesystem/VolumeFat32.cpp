/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include "VolumeFat32.h"

#include <algorithm>
#include <array>
#include "kstd.h"

using std::array;
using kstd::vector;
using kstd::string;
using kstd::rtrim;

namespace filesystem {

VolumeFat32::VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors) :
        hdd(hdd),
        bootable(bootable),
        partition_offset_in_sectors(partition_offset_in_sectors),
        partition_size_in_sectors(partition_size_in_sectors) {

    hdd.read28(partition_offset_in_sectors, &vbr, sizeof(vbr));
    fat_start = partition_offset_in_sectors + vbr.reserved_sectors;
    data_start = fat_start + vbr.fat_table_size * vbr.fat_table_copies;
}

void VolumeFat32::print_volume_info(ScreenPrinter& printer) const {
    string label = rtrim(vbr.volume_label, sizeof(vbr.volume_label));
    string software = rtrim (vbr.software_name, sizeof(vbr.software_name));
    string fat_type = rtrim(vbr.fat_type_label, sizeof(vbr.fat_type_label));
    printer.format("%, bootable: %, size: %MB, cluster size: %, software: %, fat type: %\n",
            label,
            bootable,
            partition_size_in_sectors * 512 / 1024 / 1024,
            vbr.sectors_per_cluster,
            software,
            fat_type);
}

/**
 * @brief   Get file/directory entry for given path
 * @param   unix_path Absolute path to file/directory Eg.
 *          "/home/docs/myphoto.jpg"
 *          "/home/music/"
 *          "/home/music"
 *          "/./" and "/../" and "//" in path are also supported, eg
 *          "/home/music/..
 * @return  True if entry exists, False otherwise
 */
bool VolumeFat32::get_entry_for_path(const kstd::string& unix_path, SimpleDentryFat32& out_entry) const {
    if (unix_path.empty() || unix_path.front() != '/')
        return false;

    // start at root...
    SimpleDentryFat32 e = get_root_dentry();

    // ...and descend down the path to the very last entry
    auto segments = kstd::split_string<vector<string>>(unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e.is_directory)
            return false;   // path segment is not a directory. this is error

        if (!get_entry_for_name(e, path_segment, e))
            return false;   // path segment does not exist. this is error
    }

    // managed to descend to the very last element of the path, means element found
    out_entry = e;
    return true;
}


/**
 * @brief   Enumerate directory contents
 * @param   dentry Directory entry for which we want to enumerate elements
 * @param   on_entry Callback called for every valid element in the directory
 * @return  True if all entries have been enumerated,
 *          False if enumeration stopped by on_entry() returning false
 */
bool VolumeFat32::enumerate_directory(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry) const {
    array<DirectoryEntryFat32, 16> entries;
    u32 cluster = dentry.cluster_no;

    while (cluster >= CLUSTER_FIRST_VALID && cluster != CLUSTER_END_OF_DIRECTORY) { // iterate cluster chain
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

            // read 1 sector of data (512 bytes)
            read_fat_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

            for (const auto& e : entries) { // iterate directory entries
                if (e.name[0] == DIR_ENTRY_NO_MORE)
                    return true;    // no more entries for this dir

                if (e.name[0] == DIR_ENTRY_UNUSED)
                    continue;       // unused entry, skip

                if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                    continue;       // partition label

                if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                    continue;   // extension for 8.3 filename

                if (!on_entry(make_simple_dentry(e)))
                    return false;
            }
        }
        cluster = get_next_cluster(cluster);
    }
    return true; // all entries enumerated
}

/**
 * @brief   Read maximum of count bytes into data buffer
 * @param   file  Directory entry to read from
 * @param   data  Buffer that has at least [count] capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::read_file(const SimpleDentryFat32& file, void* data, u32 count) const {
    u32 total_bytes_read = 0;
    u32 remaining_size = (count < file.size) ? count : file.size; // read the min of (count, file size)
    u32 cluster = file.cluster_no;
    u8* dst = (u8*)data;

    while (cluster >= CLUSTER_FIRST_VALID && cluster < CLUSTER_END_OF_FILE && remaining_size > 0) {
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) {
            u16 read_count = remaining_size >= 512 ? 512 : remaining_size;
            read_fat_data_sector(cluster, sector_offset, dst, read_count);
            remaining_size -= read_count;
            total_bytes_read += read_count;
            dst += read_count;
            if (remaining_size == 0)
                break;
        }

        cluster = get_next_cluster(cluster);
    }
    return total_bytes_read;
}

/**
 * @brief   Return root directory entry; this is the entry point to entire volume dir tree
 */
SimpleDentryFat32 VolumeFat32::get_root_dentry() const {
    return SimpleDentryFat32("/", 0, true, vbr.root_cluster);
}

/**
 * @brief   Get directory entry for directory pointed by dentry_cluster
 * @param   Filename Entry name. Case sensitive
 * @return  True if entry found, False otherwise
 */
bool VolumeFat32::get_entry_for_name(const SimpleDentryFat32& dentry, const kstd::string& filename, SimpleDentryFat32& out_entry) const {
    auto on_entry = [&filename, &out_entry](const SimpleDentryFat32& e) -> bool {
        if (e.name == filename) {
            out_entry = e;
            return false;   // entry found. stop enumeration
        } else
            return true;    // continue searching for entry
    };

    return !enumerate_directory(dentry, on_entry); // enumeration not completed means entry found
}

u32 VolumeFat32::get_next_cluster(u32 current_cluster) const {
    const u8 FAT_ENTRIES_PER_SECTOR = 512 / sizeof(u32); // makes 512 bytes so 1 sector
    u32 fat_buffer[FAT_ENTRIES_PER_SECTOR];

    u32 fat_sector_for_current_cluster = current_cluster / FAT_ENTRIES_PER_SECTOR;
    read_fat_table_sector(fat_sector_for_current_cluster, fat_buffer, sizeof(fat_buffer));
    u32 fat_offset_in_sector_for_current_cluster = current_cluster % FAT_ENTRIES_PER_SECTOR;

    return fat_buffer[fat_offset_in_sector_for_current_cluster] & FAT32_CLUSTER_28BIT_MASK;
}

bool VolumeFat32::read_fat_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const {
    return hdd.read28(data_start + vbr.sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
}

bool VolumeFat32::read_fat_table_sector(u32 sector, void* data, u32 size) const {
    return hdd.read28(fat_start + sector, data, size);
}

SimpleDentryFat32 VolumeFat32::make_simple_dentry(const DirectoryEntryFat32& dentry) const {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return SimpleDentryFat32(
                ext.empty() ? name : name + "." + ext,
                dentry.size,
                (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
                dentry.first_cluster_hi << 16 | dentry.first_cluster_lo
            );

}

} /* namespace filesystem */
