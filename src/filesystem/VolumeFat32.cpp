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

void VolumeFat32::print_root_tree(ScreenPrinter& printer) const {
    print_dir_tree(vbr.root_cluster, printer, 1);
//    DirectoryEntryFat32 e;
//    if (!get_entry_for_path("/LEVEL1/LEVEL2/LEVEL3/LEVEL3.TXT", e))
//        printer.format("Not found\n");
//    else if (is_directory(e))
//        printer.format("Directory\n");
//    else
//        print_file(e.first_cluster_hi << 16 | e.first_cluster_lo, e.size, printer);
}

static const char* INDENTS[] = {
        "",
        "  ",
        "    ",
        "      ",
        "        ",
        "          ",
        "            ",
        "              ",
        "                ",
        "                  "};



void VolumeFat32::print_dir_tree(u32 dentry_cluster, ScreenPrinter& printer, u8 level) const {

    auto on_entry = [&](const DirectoryEntryFat32& e, const kstd::string& full_name) -> bool {
        if (full_name == "." || full_name == "..") // skip . and ..
            return true;

        if (is_directory(e)) {
            printer.format("%[%]\n", INDENTS[level], full_name);
            print_dir_tree(e.first_cluster_hi << 16 | e.first_cluster_lo, printer, level+1);
        } else
            printer.format("%% - %B\n", INDENTS[level], full_name, e.size);

        return true;
    };

    enumerate_dentry(dentry_cluster, on_entry);
}

void VolumeFat32::print_file(u32 file_cluster, u32 file_size, ScreenPrinter& printer) const {
    u32 remaining_size = file_size;
    u8 buffer[513]; // 513 for '\0'

    while (file_cluster != CLUSTER_END_OF_FILE && remaining_size > 0) {
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) {
            u16 read_count = remaining_size >= 512 ? 512 : remaining_size;
            read_fat_data_sector(file_cluster, sector_offset, buffer, read_count);
            buffer[read_count] = '\0';
            printer.format("%", (char*) buffer);
            remaining_size -= read_count;
            if (remaining_size == 0)
                break;
        }

        file_cluster = get_next_cluster(file_cluster);
    }
}

/**
 * @brief   Get directory entry for given path
 * @param   unix_path Absolute path to directory entry Eg.
 *          "/home/docs/myphoto.jpg"
 *          "/home/music/"
 *          "/home/music"
 *          "/./" and "/../" and "//" in path are also supported, eg
 *          "/home/music/..
 * @return  True if entry found, False otherwise
 */
bool VolumeFat32::get_entry_for_path(const kstd::string& unix_path, DirectoryEntryFat32& out_entry) const {
    if (unix_path.empty() || unix_path.front() != '/')
        return false;

    // start at root...
    DirectoryEntryFat32 e = get_root_dentry();

    // ...and descend down the path to the very last entry
    auto segments = kstd::split_string<vector<string>>(unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!is_directory(e))
            return false;   // path segment is not a directory. this is error

        if (!get_entry_for_name(e.first_cluster_hi << 16 | e.first_cluster_lo, path_segment, e))
            return false;   // path segment does not exist. this is error
    }

    // managed to descend to the very last element of the path, means element found
    out_entry = e;
    return true;
}

/**
 * @brief   Get directory entry for directory pointed by dentry_cluster
 * @param   Filename Entry name. Case sensitive
 * @return  True if entry found, False otherwise
 */
bool VolumeFat32::get_entry_for_name(u32 dentry_cluster, const kstd::string& filename, DirectoryEntryFat32& out_entry) const {
    auto on_entry = [&filename, &out_entry](const DirectoryEntryFat32& e, const kstd::string& full_name) -> bool {
        if (full_name == filename) {
            out_entry = e;
            return false;   // entry found. stop enumeration
        } else
            return true;    // continue searching for entry
    };

    return !enumerate_dentry(dentry_cluster, on_entry); // enumeration not finished means entry found
}

/**
 *
 * @param   dentry_cluster Cluster od directory entry for which we want to enumerate elements
 * @param   on_entry_found Callback called for every element in the directory
 * @return  True if all entries have been enumerated,
 *          False if enumeration broke by on_entry_found() returning false
 */
bool VolumeFat32::enumerate_dentry(u32 dentry_cluster, const OnEntryFound& on_entry_found) const {
    array<DirectoryEntryFat32, 16> entries;
    while (dentry_cluster != CLUSTER_END_OF_CHAIN) { // iterate cluster chain
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

            // read 1 sector of data (512 bytes)
            read_fat_data_sector(dentry_cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

            for (const auto& e : entries) { // iterate directory entries
                if (e.name[0] == DIR_ENTRY_NO_MORE)
                    return true;    // no more entries for this dir

                if (e.name[0] == DIR_ENTRY_UNUSED)
                    continue;       // unused entry, skip

                if ((e.attributes & ATTR_VOLUMEID) == ATTR_VOLUMEID)
                    continue;       // partition label

                if ((e.attributes & ATTR_LONGNAME) == ATTR_LONGNAME)
                    continue;   // extension for 8.3 filename

                string name = rtrim(e.name, sizeof(e.name));
                string ext = rtrim(e.ext, sizeof(e.ext));
                string full_name = ext.empty() ? name : name + "." + ext;

                if (!on_entry_found(e, full_name))
                    return false;
            }
        }
        dentry_cluster = get_next_cluster(dentry_cluster);
    }
    return true; // all entries enumerated
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

bool VolumeFat32::is_directory(const filesystem::DirectoryEntryFat32& e) const {
    return (e.attributes & ATTR_DIRECTORY) == ATTR_DIRECTORY;
}

DirectoryEntryFat32 VolumeFat32::get_root_dentry() const {
    DirectoryEntryFat32 e;
    e.first_cluster_hi = vbr.root_cluster >> 16;
    e.first_cluster_lo = vbr.root_cluster & 0xFF;
    e.attributes = ATTR_DIRECTORY;
    return e;
}


} /* namespace filesystem */
