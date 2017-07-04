/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include "VolumeFat32.h"

#include <algorithm>
#include "kstd.h"

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
    DirectoryEntryFat32 entries[16];
    while (dentry_cluster != CLUSTER_END_OF_CHAIN) { // iterate cluster chain
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

            // read 1 sector of data (512 bytes)
            read_fat_data_sector(dentry_cluster, sector_offset, entries, sizeof(entries));

            for (u8 i = 0; i < 16; i++) { // iterate directory entries
                auto& e = entries[i];
                if (e.name[0] == DIR_ENTRY_NO_MORE)
                    return;     // no more entries for this dir

                if (e.name[0] == DIR_ENTRY_UNUSED)
                    continue;   // unused entry, skip

                if ((e.attributes & ATTR_VOLUMEID) == ATTR_VOLUMEID)
                    continue;   // partition label

                if ((e.attributes & ATTR_LONGNAME) == ATTR_LONGNAME)
                    continue;   // extension for 8.3 filename

                string name = rtrim(e.name, sizeof(e.name));
                string ext = rtrim(e.ext, sizeof(e.ext));
                string full_name = ext.empty() ? name : name + "." + ext;

                if ((e.attributes & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
                    if (name == "." || name == "..")
                        continue;   // skip "." and ".." entries

                    u32 clust = e.first_cluster_hi << 16 | e.first_cluster_lo;
                    printer.format("%[%] - cluster no. %\n", INDENTS[level], full_name, clust);
                    print_dir_tree(clust, printer, level + 1);
                }
                else {
                    printer.format("%% - %B ", INDENTS[level], full_name, e.size, e.attributes);
                    u32 first_file_cluster = e.first_cluster_hi << 16 | e.first_cluster_lo;
                    if (e.size > 0 && e.size < 60)
                        print_file(first_file_cluster, e.size, printer);
                    else
                        printer.format("\n");
                }
            }
        }
        dentry_cluster = get_next_cluster(dentry_cluster);
    }
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
            if (remaining_size <= 0)
                break;
        }

        file_cluster = get_next_cluster(file_cluster);
    }
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

} /* namespace filesystem */
