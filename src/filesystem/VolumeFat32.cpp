/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */


#include <algorithm>
#include <array>
#include "VolumeFat32.h"
#include "ScreenPrinter.h"
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
    data_start = fat_start + vbr.fat_table_size_in_sectors * vbr.fat_table_copies;

//    if (vbr.bytes_per_sector != 512) ERROR;
}

string VolumeFat32::get_label() const {
    return rtrim(vbr.volume_label, sizeof(vbr.volume_label));
}

string VolumeFat32::get_type() const {
    return rtrim(vbr.fat_type_label, sizeof(vbr.fat_type_label));
}

u32 VolumeFat32::get_size_in_bytes() const {
    return partition_size_in_sectors * vbr.bytes_per_sector;
}

u32 VolumeFat32::get_used_space_in_bytes() const {
    u32 table[128];
    u32 used_clusters = 0;

    for (u32 sector = 0; sector < vbr.fat_table_size_in_sectors; sector++) {
        read_fat_table_sector(sector, table, sizeof(table));
        for (u32 j = 0; j < 128; j++) {
            if (sector == 0 && j < CLUSTER_FIRST_VALID)
                continue; // first two entries in FAT are reserved just as first two data clusters and not accounted here

            u32 cluster = table[j] & FAT32_CLUSTER_28BIT_MASK;
            if (cluster != CLUSTER_UNUSED)
                used_clusters++;
        }
    }

    return used_clusters;// * vbr.sectors_per_cluster * vbr.bytes_per_sector;
}

string extract_file_name(const string& filename) {
    auto pivot = filename.rfind('/');
    return filename.substr(pivot+1, filename.size());
}

string extract_file_directory(const string& filename) {
    auto pivot = filename.rfind('/');
    return filename.substr(0, pivot+1);
}

void VolumeFat32::clear_cluster_chain_in_fat_table(u32 cluster) const {
    const u8 FAT_ENTRIES_PER_SECTOR = 512 / sizeof(u32); // makes 512 bytes so 1 sector
    u32 fat_buffer[FAT_ENTRIES_PER_SECTOR];
    while (cluster >= CLUSTER_FIRST_VALID && cluster != CLUSTER_END_OF_DIRECTORY) {
        u32 fat_sector_for_current_cluster = cluster / FAT_ENTRIES_PER_SECTOR;
        read_fat_table_sector(fat_sector_for_current_cluster, fat_buffer, sizeof(fat_buffer));
        u32 fat_offset_in_sector_for_current_cluster = cluster % FAT_ENTRIES_PER_SECTOR;
        u32 next_cluster = fat_buffer[fat_offset_in_sector_for_current_cluster] & FAT32_CLUSTER_28BIT_MASK;

        fat_buffer[fat_offset_in_sector_for_current_cluster] = CLUSTER_UNUSED; // clear cluster in fat table
        write_fat_table_sector(fat_sector_for_current_cluster, fat_buffer, sizeof(fat_buffer));
        cluster = next_cluster;
    }
}

void VolumeFat32::detach_cluster(const SimpleDentryFat32& dentry, u32 cluster_no) const {
    // set cluster as not used
    clear_cluster_chain_in_fat_table(cluster_no);
}

void VolumeFat32::cleanup_dir_cluster(const SimpleDentryFat32& e, u32 cluster) const {
    auto on_entry = [](const SimpleDentryFat32& e) -> bool {
        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        return false; // file found, stop enumeration
    };

    if (enumerate_directory_cluster(cluster, on_entry) == EnumerateResult::ENUMERATION_STOPPED)
        return; // cluster has more files, nothing to cleanup

    if (cluster == e.data_cluster) { // is this the only one cluster of the dir just emptied?
        clear_cluster_chain_in_fat_table(e.data_cluster);
        array<DirectoryEntryFat32, 16> entries;
        read_fat_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
        entries[e.entry_index].first_cluster_lo = 0; entries[e.entry_index].first_cluster_hi = 0; // mark entry as deleted
        write_fat_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    } else { // not last cluster

        // find one cluster before "cluster"
        u32 prev_cluster;
        u32 curr_cluster = e.data_cluster;
        do {
            prev_cluster = curr_cluster;
            curr_cluster = get_next_cluster(curr_cluster);

        } while (curr_cluster != cluster);

        write_fat_table_for_cluster(prev_cluster, CLUSTER_END_OF_DIRECTORY);
        write_fat_table_for_cluster(curr_cluster, CLUSTER_UNUSED);

    }

}

bool VolumeFat32::write_fat_table_for_cluster(u32 cluster, u32 value) const {
    if (!(cluster >= CLUSTER_FIRST_VALID && cluster < CLUSTER_END_OF_DIRECTORY))
        return true;

    const u8 FAT_ENTRIES_PER_SECTOR = 512 / sizeof(u32); // makes 512 bytes so 1 sector
    u32 fat_buffer[FAT_ENTRIES_PER_SECTOR];

    u32 fat_sector_for_cluster = cluster / FAT_ENTRIES_PER_SECTOR;
    if (!read_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    u32 fat_offset_in_sector_for_current_cluster = cluster % FAT_ENTRIES_PER_SECTOR;

    fat_buffer[fat_offset_in_sector_for_current_cluster] = value;
    if (!write_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    return true;
}

void VolumeFat32::mark_entry_unused(const SimpleDentryFat32& e) const {
    // 1. mark entry as UNUSED
    array<DirectoryEntryFat32, 16> entries;
    read_fat_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_UNUSED; // mark entry as deleted
    write_fat_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

bool VolumeFat32::delete_file(const string& filename) const {
    ScreenPrinter& printer = ScreenPrinter::instance();

    SimpleDentryFat32 e;
    if (!get_entry_for_path(filename, e))
        return false;

    string parent_dir = extract_file_directory(filename);
    SimpleDentryFat32 parent_e;
    get_entry_for_path(parent_dir, parent_e);

    if (e.is_directory) {
        if (e.data_cluster != CLUSTER_UNUSED) return false; // can only delete empty directory

        // 1. mark entry as UNUSED
        mark_entry_unused(e);

        // 3. cleanup parent dir eg detach unused cluster if this was last entry
        cleanup_dir_cluster(parent_e, e.entry_cluster);
    }
    else {
        // 1. mark entry as UNUSED
        mark_entry_unused(e);

        // 2. clear data cluster chain
        clear_cluster_chain_in_fat_table(e.data_cluster);

        // 3. cleanup parent dir eg detach unused cluster if this was last entry
        cleanup_dir_cluster(parent_e, e.entry_cluster);
    }
    /*
    string name = extract_file_name(filename);
    string path = extract_file_directory(filename);
    SimpleDentryFat32 dentry;
    if (!get_entry_for_path(path, dentry))
        return false; // directory not exists

    if (!dentry.is_directory)
        return false; // parent must be directory
*/



    /*
    array<DirectoryEntryFat32, 16> entries;
    u32 cluster = dentry.data_cluster;
    u32 prev_cluster = cluster;
    while (cluster >= CLUSTER_FIRST_VALID && cluster != CLUSTER_END_OF_DIRECTORY) { // iterate cluster chain
        u32 num_entries_in_dir = 0;
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

            // read 1 sector of data (512 bytes)
            read_fat_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

            for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
                auto& e = entries[i];
                if (e.name[0] == DIR_ENTRY_NO_MORE)
                    return false;    // no more entries for this dir

                if (e.name[0] == DIR_ENTRY_UNUSED)
                    continue;       // unused entry, skip

                if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                    continue;       // partition label

                if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                    continue;   // extension for 8.3 filename



                SimpleDentryFat32 simple_e = make_simple_dentry(e, cluster, sector_offset, i);
                if (simple_e.name == name) { // file found
                    if (simple_e.is_directory && simple_e.data_cluster != CLUSTER_UNUSED)
                        return false; // can only delete empty directory

                    // 1. set file entry to unused
                    e.name[0] = DIR_ENTRY_UNUSED;
                    write_fat_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

                    // 2. clear sector chain in fat table
                    u32 e_cluster = simple_e.data_cluster;
                   // printer.format("e_cluster: %\n", e_cluster);
                    clear_cluster_chain_in_fat_table(e_cluster);
                    entry_deleted = true;
                } else
                    num_entries_in_dir++;
            }
        }
        if (num_entries_in_dir == 0) { //detach cluster
            clear_cluster_chain_in_fat_table(cluster);
            detach_cluster(dentry, cluster);
        }
        prev_cluster = cluster;
        cluster = get_next_cluster(cluster);
    }*/
    return true;
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
bool VolumeFat32::get_entry_for_path(const string& unix_path, SimpleDentryFat32& out_entry) const {
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

    u32 cluster = dentry.data_cluster;

    while (cluster >= CLUSTER_FIRST_VALID && cluster < CLUSTER_END_OF_DIRECTORY) { // iterate cluster chain
        switch (enumerate_directory_cluster(cluster, on_entry)) {
        case EnumerateResult::ENUMERATION_STOPPED :
            return false;
        case EnumerateResult::ENUMERATION_FINISHED :
            return true;
        default: ;
        }
        cluster = get_next_cluster(cluster);
    }
    return true; // all entries enumerated
}

VolumeFat32::EnumerateResult VolumeFat32::enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry) const {
    array<DirectoryEntryFat32, 16> entries;

    for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (512 bytes)
        read_fat_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
            const auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE)
                return EnumerateResult::ENUMERATION_FINISHED;    // no more entries for this dir

            if (e.name[0] == DIR_ENTRY_UNUSED)
                continue;       // unused entry, skip

            if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                continue;       // partition label

            if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                continue;   // extension for 8.3 filename

            if (!on_entry(make_simple_dentry(e, cluster, sector_offset, i)))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE;
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
    u32 cluster = file.data_cluster;
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
    return SimpleDentryFat32("/", 0, true, vbr.root_cluster, 0, 0, 0);
}

/**
 * @brief   Get directory entry for directory pointed by dentry_cluster
 * @param   Filename Entry name. Case sensitive
 * @return  True if entry found, False otherwise
 */
bool VolumeFat32::get_entry_for_name(const SimpleDentryFat32& dentry, const string& filename, SimpleDentryFat32& out_entry) const {
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

bool VolumeFat32::write_fat_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const {
    return hdd.write28(data_start + vbr.sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
}

bool VolumeFat32::read_fat_table_sector(u32 sector, void* data, u32 size) const {
    return hdd.read28(fat_start + sector, data, size);
}

bool VolumeFat32::write_fat_table_sector(u32 sector, void const* data, u32 size) const {
    return hdd.write28(fat_start + sector, data, size);
}

SimpleDentryFat32 VolumeFat32::make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) const {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return SimpleDentryFat32(
                ext.empty() ? name : name + "." + ext,
                dentry.size,
                (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
                dentry.first_cluster_hi << 16 | dentry.first_cluster_lo,
                entry_cluster,
                entry_sector,
                entry_index
            );

}

} /* namespace filesystem */
