/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */


#include <algorithm>
#include "VolumeFat32.h"
#include "ScreenPrinter.h"
#include "kstd.h"

using kstd::vector;
using kstd::string;
using kstd::rtrim;

namespace filesystem {

static ScreenPrinter& printer = ScreenPrinter::instance();

VolumeFat32::VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors) :
        hdd(hdd),
        bootable(bootable),
        partition_offset_in_sectors(partition_offset_in_sectors),
        partition_size_in_sectors(partition_size_in_sectors),
        fat_table(hdd),
        fat_data(hdd) {

    hdd.read28(partition_offset_in_sectors, &vbr, sizeof(vbr));

    fat_start = partition_offset_in_sectors + vbr.reserved_sectors;
    fat_table.setup(vbr.bytes_per_sector, fat_start, vbr.fat_table_size_in_sectors);

    data_start = fat_start + vbr.fat_table_size_in_sectors * vbr.fat_table_copies;
    fat_data.setup(data_start, vbr.sectors_per_cluster);
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
    return get_used_space_in_clusters() * vbr.sectors_per_cluster * vbr.bytes_per_sector;
}

u32 VolumeFat32::get_used_space_in_clusters() const {
    return fat_table.get_used_space_in_clusters();
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
bool VolumeFat32::get_entry(const string& unix_path, SimpleDentryFat32& out_entry) const {
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
 * @brief   Read maximum of count bytes into data buffer
 * @param   file  Directory entry to read from
 * @param   data  Buffer that has at least [count] capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::read_file_entry(const SimpleDentryFat32& file, void* data, u32 count) const {
    const u16 SECTOR_SIZE = vbr.bytes_per_sector;
    u32 total_bytes_read = 0;
    u32 remaining_size = (count < file.size) ? count : file.size; // read the min of (count, file size)
    u32 cluster = file.data_cluster;
    u8* dst = (u8*)data;

    while (fat_table.is_allocated_cluster(cluster) && remaining_size > 0) {
        for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) {
            u16 read_count = remaining_size >= SECTOR_SIZE ? SECTOR_SIZE : remaining_size;
            fat_data.read_data_sector(cluster, sector_offset, dst, read_count);
            remaining_size -= read_count;
            total_bytes_read += read_count;
            dst += read_count;
            if (remaining_size == 0)
                break;
        }

        cluster = fat_table.get_next_cluster(cluster);
    }
    return total_bytes_read;
}

/**
 * @brief   Enumerate directory contents
 * @param   dentry Directory entry for which we want to enumerate elements
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
EnumerateResult VolumeFat32::enumerate_directory_entry(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry) const {
    u32 cluster = dentry.data_cluster;

    while (fat_table.is_allocated_cluster(cluster)) { // iterate cluster chain
        switch (fat_data.enumerate_directory_cluster(cluster, on_entry)) {
        case EnumerateResult::ENUMERATION_STOPPED:
            return EnumerateResult::ENUMERATION_STOPPED;
        case EnumerateResult::ENUMERATION_FINISHED:
            return EnumerateResult::ENUMERATION_FINISHED;
        default:
            // EnumerateResult::ENUMERATION_CONTINUE
            cluster = fat_table.get_next_cluster(cluster);
        }
    }
    return EnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

bool VolumeFat32::create_entry(const kstd::string& unix_path, bool directory) const {
    if (unix_path.empty() || unix_path == "/")
        return false;

    // check if parent_dir exists
    SimpleDentryFat32 parent_dir;
    if (!get_entry(extract_file_directory(unix_path), parent_dir))
        return false;

//     check if entry already exists
//    make_8_3_filename
    SimpleDentryFat32 tmp;
    string name_8_3;
    if (get_entry_for_name(parent_dir, name_8_3, tmp))
        return false;   // entry exists

    SimpleDentryFat32 e;
    e.name = extract_file_name(unix_path);
    e.is_directory = directory;

    printer.format("Creating entry. Name: %\n", e.name);
    return alloc_entry_in_directory(parent_dir, e);
}

/**
 * @brief   Delete file or empty dir. Cant delete root "/"
 * @return  True on successful deletion, False if:
 *          -empty path specified
 *          -root dir specified
 *          -path does not exist
 *          -path points to a dir that is not empty
 */
bool VolumeFat32::delete_entry(const string& unix_path) const {
    if (unix_path.empty() || unix_path == "/")
        return false;

    // get entry to be deleted
    SimpleDentryFat32 e;
    if (!get_entry(unix_path, e))
        return false;

    // get entry parent dir to be updated. If we got here, it must exist
    SimpleDentryFat32 parent_dir;
    get_entry(extract_file_directory(unix_path), parent_dir);

    // take action dependent on whether we delete file or directory
    if (e.is_directory) {
        // 1. can only delete empty directory
        if (!is_directory_empty(e))
            return false;

        // 2. mark dir entry as UNUSED
        fat_data.release_entry(e);

        // 3. if cluster contains no more files - remove it from the chain
        remove_dir_cluster_if_empty(parent_dir, e.entry_cluster);
    }
    else {
        // 1. clear file data cluster chain
        fat_table.free_cluster_chain(e.data_cluster);

        // 2. mark dir entry as UNUSED
        fat_data.release_entry(e);

        // 3. if cluster contains no more files - remove it from the chain
        remove_dir_cluster_if_empty(parent_dir, e.entry_cluster);
    }

    return true;
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

    return enumerate_directory_entry(dentry, on_entry) == EnumerateResult::ENUMERATION_STOPPED; // enumeration stopped means entry found
}

void VolumeFat32::remove_dir_cluster_if_empty(const SimpleDentryFat32& parent_dir, u32 cluster) const {
    if (!fat_data.is_directory_cluster_empty(cluster))
        return; // cluster has files; dont touch it

    // first, remember next cluster no in the chain
    u32 next_cluster = fat_table.get_next_cluster(cluster);

    // removing first cluster? update head
    if (cluster == parent_dir.data_cluster) {
        u32 directory_first_cluster = (next_cluster == Fat32Table::CLUSTER_END_OF_DIRECTORY) ? Fat32Table::CLUSTER_UNUSED : next_cluster;

        // update head
        fat_data.set_entry_data_cluster(parent_dir, directory_first_cluster);
    }
    // removing not first cluster? update previous cluster to point to the next cluster effectively removing current link
    else {
        // find one cluster before "cluster" and link it with next_cluster, detaching cluster
        u32 prev_cluster = fat_table.get_prev_cluster(parent_dir.data_cluster, cluster);
        fat_table.set_next_cluster(prev_cluster, next_cluster);
    }
    fat_table.set_next_cluster(cluster, Fat32Table::CLUSTER_UNUSED);    // this cluster is free
}

bool VolumeFat32::alloc_entry_in_directory(const SimpleDentryFat32& dir, SimpleDentryFat32 &e) const {
    if (dir.data_cluster == Fat32Table::CLUSTER_UNUSED) {
        u32 cluster = fat_table.alloc_cluster_for_directory();
        if (cluster == Fat32Table::CLUSTER_UNUSED)
            return false; // cluster allocation failed


        // TODO: first make sure entry with such name doesnt exist

        // attach data cluster to directory
        fat_data.set_entry_data_cluster(dir, cluster);

        // setup
        e.data_cluster = Fat32Table::CLUSTER_UNUSED;
        e.entry_cluster = cluster;
        e.entry_sector = 0;
        e.entry_index = 0;
        e.size = 0;

        fat_data.write_entry(e);
        return true;
    }

    return false;
}

bool VolumeFat32::is_directory_empty(const SimpleDentryFat32& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}

} /* namespace filesystem */
