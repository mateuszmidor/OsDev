/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "VolumeFat32.h"
#include "Fat32Utils.h"

using std::array;
using namespace kstd;
using namespace utils;
namespace filesystem {

/**
 * Constructor.
 * @brief   Create a volume based on physical drive device, and partition offset
 * @param   hdd Physical device this volume is located on
 * @param   bootable Is this OS bootable volume?
 * @param   partition_offset_in_sectors Where the volume data starts on the device
 * @param   partition_size_in_sectors How big the volume is
 */
VolumeFat32::VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors) :
        hdd(hdd),
        bootable(bootable),
        partition_offset_in_sectors(partition_offset_in_sectors),
        partition_size_in_sectors(partition_size_in_sectors),
        fat_table(hdd),
        fat_data(hdd),
        klog(KernelLog::instance()) {

    hdd.read28(partition_offset_in_sectors, &vbr, sizeof(vbr));

    u32 fat_start = partition_offset_in_sectors + vbr.reserved_sectors;
    fat_table.setup(fat_start, vbr.bytes_per_sector,  vbr.sectors_per_cluster, vbr.fat_table_size_in_sectors);

    u32 data_start = fat_start + vbr.fat_table_size_in_sectors * vbr.fat_table_copies;
    fat_data.setup(data_start, vbr.bytes_per_sector, vbr.sectors_per_cluster);
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
 *          "/./" and "/../" and "//" in path are also supported as they are part of Fat32 format, eg
 *          "/home/music/..
 * @return  Valid entry if exists, empty entry otherwise
 */
Fat32Entry VolumeFat32::get_entry(const string& unix_path) const {
    if (unix_path.empty() || unix_path.front() != '/')
        return empty_entry();

    // start at root...
    Fat32Entry e = get_root_dentry();

    // ...and descend down the path to the very last entry
    auto normalized_unix_path = Fat32Utils::normalize_path(unix_path); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e.is_directory) {
            klog.format("VolumeFat32::get_entry: entry '%' is not a directory\n", e.name);
            return empty_entry();   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VolumeFat32::get_entry: entry '%' does not exist\n", path_segment);
            return empty_entry();   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return e;
}

Fat32Entry VolumeFat32::empty_entry() const {
    return Fat32Entry(fat_table, fat_data);
}

/**
 * @brief   Create new file/directory for given unix path
 * @return  Valid entry if created successfully, empty entry otherwise
 */
Fat32Entry VolumeFat32::create_entry(const kstd::string& unix_path, bool directory) const {
    if (unix_path.empty() || unix_path == "/")
        return empty_entry();

    // check if entry name specified
    string name = extract_file_name(unix_path);
    if (name.empty()) {
        klog.format("VolumeFat32::create_entry: Please specify name for entry to create: %\n", unix_path);
        return empty_entry();
    }

    // check if parent_dir exists
    auto parent_dir = get_entry(extract_file_directory(unix_path));
    if (!parent_dir) {
        klog.format("VolumeFat32::create_entry: Parent not exists: %\n", extract_file_directory(unix_path));
        return empty_entry();
    }

    // check if entry already exists
    string name_8_3 = Fat32Utils::make_8_3_filename(name);
    Fat32Entry tmp = get_entry_for_name(parent_dir, name_8_3);
    if (tmp) {
        klog.format("VolumeFat32::create_entry: Entry already exists: %(%)\n", name_8_3, unix_path);
        return empty_entry();
    }

    // allocate entry in parent_dir
    auto out = empty_entry();
    out.name = name;
    out.is_directory = directory;
    klog.format("VolumeFat32::create_entry: name is %\n", out.name);
    parent_dir.alloc_entry_in_directory(out);
    return out;
}

/**
 * @brief   Delete file or empty directory
 * @return  True on successful deletion, False if:
 *          -empty path specified
 *          -root dir specified
 *          -path does not exist
 *          -path points to a dir that is not empty
 */
bool VolumeFat32::delete_entry(const string& unix_path) const{
    if (unix_path.empty() || unix_path == "/")
        return false;

    // get entry parent dir to be updated
    auto parent_dir = get_entry(extract_file_directory(unix_path));
    if (!parent_dir) {
        klog.format("VolumeFat32::delete_entry: path doesn't exist: %\n", unix_path);
        return false;
    }

    // get entry to be deleted
    Fat32Entry e = get_entry_for_name(parent_dir, extract_file_name(unix_path));
    if (!e) {
        klog.format("VolumeFat32::delete_entry: path doesn't exist: %\n", unix_path);
        return false;
    }

    // for directory - ensure it is empty
    if (e.is_directory && !is_directory_empty(e)) {
        klog.format("VolumeFat32::delete_entry: can't delete non-empty directory: %\n", unix_path);
        return false;
    }

    // for file - release its data
    if (!e.is_directory)
        e.data.free();

    // mark entry as nomore/unused depending on it's position in the directory
    if (is_no_more_entires_after(parent_dir, e)) {
        klog.format("VolumeFat32::delete_entry: no more entries after\n");
        mark_entry_as_nomore(e);
    }
    else {
        klog.format("VolumeFat32::delete_entry: set entry unused\n");
        mark_entry_as_unused(e);
    }

    // if cluster where our entry was allocated contains no more files - remove it from the chain.
    // but dont remove root first cluster!
    u32 entry_cluster = fat_table.find_cluster_for_byte(e.parent_data.get_head(), e.parent_index * sizeof(DirectoryEntryFat32));
    if (entry_cluster != vbr.root_cluster && is_directory_cluster_empty(parent_dir, entry_cluster))
        detach_directory_cluster(parent_dir, entry_cluster);

    return true;
}

/**
 * @brief   Move file/directory within volume
 * @param   unix_path_from Exact source entry path
 * @param   unix_path_to Exact destination path/destination directory
 * @return  True on success, False otherwise
 */
bool VolumeFat32::move_entry(const kstd::string& unix_path_from, const kstd::string& unix_path_to) const {
    // get source entry
    auto src = get_entry(unix_path_from);
    if (!src) {
        klog.format("VolumeFat32::move_entry: src entry doesn't exists '%'\n", unix_path_from);
        return false;
    }

    // if destination directory specified instead of full path - keep source name
    string path_to = unix_path_to;
    auto tmp = get_entry(unix_path_to);
    if (tmp)
        if (tmp.is_directory)
            path_to += string("/") + extract_file_name(unix_path_from);

    // create destination entry
    auto dst = create_entry(path_to, src.is_directory);
    if (!dst) {
        klog.format("VolumeFat32::move_entry: can't create dst entry '%'\n", path_to);
        return false;
    }

    // move data cluster chain from src path_to dst entry
    std::swap(src.data, dst.data);
    src.write_entry();
    dst.write_entry();

    // remove src entry
    if (!delete_entry(unix_path_from)) {
        klog.format("VolumeFat32::move_entry: can't remove src entry '%'\n", unix_path_from);
        return false;
    }

    return true;
}

/**
 * @brief   Return root directory entry; this is the entry point to entire volume dir tree
 */
Fat32Entry VolumeFat32::get_root_dentry() const {
    return Fat32Entry(fat_table, fat_data, "/", 0, true, vbr.root_cluster, Fat32Table::CLUSTER_END_OF_CHAIN, 0);
}

/**
 * @brief   Get directory entry for directory pointed by dentry_cluster
 * @param   Filename Entry name. Case sensitive
 * @return  Valid entry if exists, empty entry otherwise
 */
Fat32Entry VolumeFat32::get_entry_for_name(Fat32Entry& parent_dir, const string& filename) const {
    Fat32Entry result = empty_entry();
    auto on_entry = [&](const Fat32Entry& e) -> bool {
        if (e.name == filename) {
            result = e;
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };
    parent_dir.enumerate_entries(on_entry);
    return result;
}

bool VolumeFat32::alloc_first_dir_cluster_and_alloc_entry(Fat32Entry& parent_dir, Fat32Entry& e) const {

    // setup directory head
//    if (!parent_dir.data.attach_cluster_and_zero_it())
//        return false;
//
//    // head updated; persist it
//    parent_dir.write_entry();
//
//    // setup entry
//    e.parent_data = parent_dir.data;
//    e.parent_index = 0;
//    e.write_entry();

    return true;
}

bool VolumeFat32::alloc_last_dir_cluster_and_alloc_entry(Fat32Entry& parent_dir, Fat32Entry& e) const {
//    if (!parent_dir.data.attach_cluster_and_zero_it())
//        return false;
//
//    // setup file entry
//    e.parent_data = parent_dir.data;
////    e.parent_index = ?;
//    e.write_entry();

    return true;
}

//-------------------------


bool VolumeFat32::try_alloc_entry_in_free_dir_slot(const Fat32Entry& parent_dir, Fat32Entry &e) const {
//    u32 cluster = parent_dir.data.get_head();
//    u8 entry_type = Fat32Entry::DIR_ENTRY_NOT_FOUND;
//
//    // try find free slot in direcyory clusters
//    while (fat_table.is_allocated_cluster(cluster)) {
//        entry_type = get_free_entry_in_dir_cluster(cluster, e);
//        if (entry_type != Fat32Entry::DIR_ENTRY_NOT_FOUND)
//            break; // free entry found
//
//        cluster = fat_table.get_next_cluster(cluster);
//        klog.format("   Next Cluster: %\n", cluster);
//    }
//
//    // UNUSED entry found - just reuse it
//    if (entry_type == Fat32Entry::DIR_ENTRY_UNUSED) {
//        klog.format("   Unused entry found. [Index in parent]: [%]\n", e.parent_index);
//        e.write_entry();
//        return true;
//    }
//
//    // NO_MORE entry found - reuse it and mark the next one as NO_MORE
//    if (entry_type == Fat32Entry::DIR_ENTRY_NO_MORE) {
//        klog.format("   NO MORE entry found. [Index in parent]: [%]\n", e.parent_index);
//        e.write_entry();
//        mark_next_entry_as_nomore(e);
//        return true;
//    }

    return false; // no free slot found
}

void VolumeFat32::detach_directory_cluster(Fat32Entry& parent_dir, u32 cluster) const {
    u32 old_head = parent_dir.data.get_head();
    parent_dir.data.detach_cluster(cluster);
    u32 new_head = parent_dir.data.get_head();
    if (old_head != new_head)
        parent_dir.write_entry();
}

/**
 * @brief   Check if entry is the last entry present in parent_dir
 * @return  True if there is no valid entries after our entry. False otherwise
 */
bool VolumeFat32::is_no_more_entires_after(Fat32Entry& parent_dir, const Fat32Entry& entry) const {
    klog.format("is_no_more_entires_after index %\n", entry.parent_index);
    bool entry_found = false;
    auto on_entry = [&entry, &entry_found, this](const Fat32Entry& e) -> bool {

        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        if (entry_found) {
            klog.format("Entry after deleted found: %, index %\n", e.name, e.parent_index);
            return false; // entry after our entry found, stop enumeration
        }

        if (e.parent_index == entry.parent_index)
            entry_found = true; // our entry found, mark this fact

        return true;
    };

    return (parent_dir.enumerate_entries(on_entry) == EnumerateResult::ENUMERATION_FINISHED); // finished means no more entries after entry in parent_dir
}

bool VolumeFat32::is_directory_empty(const Fat32Entry& e) const {
    return e.data.empty();
}

/**
 * @brief   Scan directory cluster looking for either UNUSED or NO_MORE entry
 * @param   out Free entry, if found, is stored here
 * @return  DIR_ENTRY_UNUSED, DIR_ENTRY_NO_MORE or 0xFF if not found
 */
u8 VolumeFat32::get_free_entry_in_dir_cluster(u32 cluster, Fat32Entry& out) const {
//    array<DirectoryEntryFat32, Fat32Entry::FAT32ENTRIES_PER_SECTOR> entries;
//
//    for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster
//
//        // read 1 sector of data (usually 512 bytes)
//        fat_data.read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
//
//        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
//            const auto& e = entries[i];
//            if (e.name[0] == Fat32Entry::DIR_ENTRY_NO_MORE || e.name[0] == Fat32Entry::DIR_ENTRY_UNUSED) {
//                out.entry_cluster = cluster;
//                out.entry_sector = sector_offset;
//                out.entry_index = i;
//                return e.name[0];
//            }
//        }
//    }
    return 0;//Fat32Entry::DIR_ENTRY_NOT_FOUND; // no free entry found in this cluster
}

void VolumeFat32::mark_entry_as_nomore(Fat32Entry& e) const {
    e.name = DirectoryEntryFat32::DIR_ENTRY_NO_MORE; // mark entry as last one
    e.write_entry();
}

void VolumeFat32::mark_next_entry_as_nomore(const Fat32Entry& e) const {
    // if e is the very last entry in the cluster - CLUSTER_END_OF_DIRECTORY will mark the end of directory entries
    if (e.parent_index == Fat32Entry::FAT32ENTRIES_PER_SECTOR * vbr.sectors_per_cluster - 1)
        return;

    Fat32Entry no_more = empty_entry();
    no_more.parent_data = e.parent_data;
    no_more.parent_index = e.parent_index + 1;
    mark_entry_as_nomore(no_more);
}

void VolumeFat32::mark_entry_as_unused(Fat32Entry& e) const {
    // TODO: will conversion of names work for this?
    e.name = DirectoryEntryFat32::DIR_ENTRY_UNUSED; // mark entry as deleted
    e.write_entry();
}

bool VolumeFat32::is_directory_cluster_empty(const Fat32Entry& directory, u32 cluster) const {
    auto on_entry = [](const Fat32Entry& e) -> bool {
        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        return false; // file found, stop enumeration
    };
    return (directory.enumerate_directory_cluster(cluster, on_entry) != EnumerateResult::ENUMERATION_STOPPED);
}

} /* namespace filesystem */
