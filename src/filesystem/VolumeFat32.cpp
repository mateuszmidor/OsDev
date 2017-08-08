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

    fat_start = partition_offset_in_sectors + vbr.reserved_sectors;
    fat_table.setup(fat_start, vbr.bytes_per_sector,  vbr.sectors_per_cluster, vbr.fat_table_size_in_sectors);

    data_start = fat_start + vbr.fat_table_size_in_sectors * vbr.fat_table_copies;
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
 * @return  Valid entry exists, Invalid entry otherwise
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

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   file File entry to write to
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 VolumeFat32::write_file_entry(Fat32Entry& file, void const* data, u32 count) const {
    if (file.entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("VolumeFat32::write_file_entry: uninitialized entry to write specified\n");
        return 0;
    }

    if (file.is_directory){
        klog.format("VolumeFat32::write_file_entry: specified entry is a directory\n");
        return 0;
    }

    if (file.position + count > 0xFFFFFFFF){
        klog.format("VolumeFat32::write_file_entry: file entry exceeds Fat32 4GB limit\n");
        return 0;
    }

    return write_file_data(file, data, count);;
}

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   file File entry to write to
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 VolumeFat32::write_file_data(Fat32Entry& file, const void* data, u32 count) const {
    // 1. setup writing status variables
    u32 total_bytes_written = 0;
    u32 remaining_bytes_to_write = count;

    // 2. locate writing start point
    u32 position = file.position;
    u32 cluster = get_cluster_for_write(file);

    // 3. follow/make cluster chain and write data to sectors until requested number of bytes is written
    const u8* src = (const u8*)data;
    while (fat_table.is_allocated_cluster(cluster)) {
        // write the cluster until end of cluster or requested number of bytes is written
        u32 count = fat_data.write_data_cluster(position, cluster, src, remaining_bytes_to_write);
        remaining_bytes_to_write -= count;
        total_bytes_written += count;

        // stop writing if requested number of bytes is written
        if (remaining_bytes_to_write == 0)
            break;

        // move on to the next cluster
        src += count;
        position = 0;
        cluster = attach_next_cluster(cluster);
    }

    // 4. done; update file position and size if needed
    file.position += total_bytes_written;
    file.position_data_cluster = cluster;

    if (file.size < file.position) {
        file.size = file.position;
        write_entry(file);
    }

    return total_bytes_written;
}

/**
 * @brief   Get proper cluster for data writing depending on file status and file position
 * @return  Cluster for writing data
 */
u32 VolumeFat32::get_cluster_for_write(Fat32Entry& file) const {
    u32 cluster;
    if (is_file_empty(file)) {                          // file has no data clusters yet - alloc and assign as first data cluster
        cluster = fat_table.alloc_cluster();
        file.data_cluster = cluster;
    } else if (fat_data.is_cluster_beginning(file.position)) {   // position points to beginning of new cluster - alloc this new cluster
        u32 last_cluster = fat_table.get_last_cluster(file.data_cluster);
        cluster = attach_next_cluster(last_cluster);
    } else {                                            // just use the current cluster as it has space in it
        cluster = file.position_data_cluster;
    }

    return cluster;
}

Fat32Entry VolumeFat32::empty_entry() const {
    return Fat32Entry(fat_table, fat_data);
}

void VolumeFat32::trunc_file_entry(Fat32Entry& file, u32 new_size) const {
    if (file.entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("VolumeFat32::trunc_file_entry: uninitialized entry specified\n");
        return;
    }

    if (file.is_directory){
        klog.format("VolumeFat32::trunc_file_entry: specified entry is a directory\n");
        return;
    }

    if (new_size == 0) {
        klog.format("truncating file to 0B\n");
        fat_table.free_cluster_chain(file.data_cluster);
        file.data_cluster = Fat32Table::CLUSTER_UNUSED;
        file.size = 0;
        // dont change file position
    } else
    if (new_size > file.size) {
         klog.format("extending file from % to %\n", file.size, new_size);
         // move to the old file end
         u32 old_position = file.position;
         file.seek(file.size);
         klog.format("seek to byte %, cluster %\n", file.size, file.position_data_cluster);
         // calc number of zeroes needed
         u32 remaining_zeroes = new_size - file.size;
         klog.format("remaining zeroes %\n", remaining_zeroes);
         // prepare zeroes
         const u16 SIZE = vbr.bytes_per_sector;
         u8 zeroes[SIZE];
         memset(zeroes, 0, SIZE);

         // fill file tail with zeroes
         while (remaining_zeroes != 0) {
             u32 count = min(remaining_zeroes, SIZE);
             klog.format("writing % zeroes \n", count);
             write_file_entry(file, zeroes, count);
             remaining_zeroes -= count;
         }
         file.seek(old_position);
         // file.size already updated by write_file_entry
         // dont change file position
    } else
    if (new_size < file.size) {
        klog.format("shrinking file from % to %\n", file.size, new_size);
        u32 last_cluster = fat_table.find_cluster_for_byte(file.data_cluster, new_size -1);
        u32 cluster_after_end = fat_table.get_next_cluster(last_cluster);
        if (fat_table.is_allocated_cluster(cluster_after_end)) {
            klog.format("freeing cluster\n");
            fat_table.free_cluster_chain(cluster_after_end);
            fat_table.set_next_cluster(last_cluster, Fat32Table::CLUSTER_END_OF_CHAIN);
        }
        file.size = new_size;
    }

    write_entry(file);
}

/**
 * @brief   Enumerate directory contents
 * @param   dentry Directory entry for which we want to enumerate elements
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
EnumerateResult VolumeFat32::enumerate_directory_entry(const Fat32Entry& dentry, const OnEntryFound& on_entry) const {
    if (dentry.entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("VolumeFat32::enumerate_directory_entry: uninitialized entry specified\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    if (!dentry.is_directory){
        klog.format("VolumeFat32::enumerate_directory_entry: specified entry is a file\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    u32 cluster = dentry.data_cluster;

    while (fat_table.is_allocated_cluster(cluster)) { // iterate cluster chain
        switch (enumerate_directory_cluster(cluster, on_entry)) {
        case EnumerateResult::ENUMERATION_STOPPED:
            return EnumerateResult::ENUMERATION_STOPPED;

        case EnumerateResult::ENUMERATION_FINISHED:
            return EnumerateResult::ENUMERATION_FINISHED;

        case EnumerateResult::ENUMERATION_CONTINUE:
        default:
            cluster = fat_table.get_next_cluster(cluster);
        }
    }
    return EnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

/**
 * @brief   Create new file/directory for given unix path
 * @return  True if entry created successfuly, False otherwise
 */
Fat32Entry VolumeFat32::create_entry(const kstd::string& unix_path, bool directory) const {
    if (unix_path.empty() || unix_path == "/")
        return empty_entry();

    // check if parent_dir exists
    auto parent_dir = get_entry(extract_file_directory(unix_path));
    if (!parent_dir) {
        klog.format("VolumeFat32::create_entry: Parent not exists: %\n", extract_file_directory(unix_path));
        return empty_entry();
    }

    // check if entry already exists
    string name = extract_file_name(unix_path);
    if (name.empty()) {
        klog.format("VolumeFat32::create_entry: Please specify name for entry to create: %\n", unix_path);
        return empty_entry();
    }

    string name_8_3 = Fat32Utils::make_8_3_filename(name);
    Fat32Entry tmp = get_entry_for_name(parent_dir, name_8_3);
    if (tmp) {
        klog.format("VolumeFat32::create_entry: Entry already exists: %(%)\n", name_8_3, unix_path);
        return Optional<Fat32Entry>(empty_entry());  // entry exists
    }

    // allocate entry in parent_dir
    auto out = empty_entry();
    out.name = name;
    out.is_directory = directory;
    alloc_entry_in_directory(parent_dir, out);
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
        fat_table.free_cluster_chain(e.data_cluster);

    // mark entry as nomore/unused depending on it's position in the directory
    if (is_no_more_entires_after(parent_dir, e))
        mark_entry_as_nomore(e);
    else
        mark_entry_as_unused(e);

    // if cluster where our entry was allocated contains no more files - remove it from the chain.
    // but dont remove root first cluster!
    if (e.entry_cluster != vbr.root_cluster && is_directory_cluster_empty(e.entry_cluster))
        detach_directory_cluster(parent_dir, e.entry_cluster);

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
    dst.data_cluster = src.data_cluster;
    src.data_cluster = Fat32Table::CLUSTER_UNUSED;
    dst.size = src.size;
    src.size = 0;
    write_entry(src);
    write_entry(dst);

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
    return Fat32Entry(fat_table, fat_data, "/", 0, true, vbr.root_cluster, Fat32Table::CLUSTER_END_OF_CHAIN, 0, 0);
}

/**
 * @brief   Get directory entry for directory pointed by dentry_cluster
 * @param   Filename Entry name. Case sensitive
 * @return  True if entry found, False otherwise
 */
Fat32Entry VolumeFat32::get_entry_for_name(const Fat32Entry& parent_dir, const string& filename) const {
    Fat32Entry result = empty_entry();
    auto on_entry = [&](const Fat32Entry& e) -> bool {
        if (e.name == filename) {
            result = e;
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };
    enumerate_directory_entry(parent_dir, on_entry);
    return result;
}

/**
 * @brief   Find empty slot in parent dir or attach new data cluster and allocate entry in it.
 *          parent_dir.data_cluster can be modified if first cluster is allocated for this directory
 *          entry cluster, segment and index are set to describe the allocated position in parent_dir
 * @return  True if entry was successfully allocated in parent_dir
 */
bool VolumeFat32::alloc_entry_in_directory(Fat32Entry& parent_dir, Fat32Entry& e) const {
    // if dir has no data cluster yet (dir is empty) - allocate new cluster and set it as directory first cluster
    if (parent_dir.data_cluster == Fat32Table::CLUSTER_UNUSED) {
        return alloc_first_dir_cluster_and_alloc_entry(parent_dir, e);
    }

    // if there is free slot in parent_dir for our entry
    if (try_alloc_entry_in_free_dir_slot(parent_dir, e))
        return true;

    // no free slot for our entry found (all entries in all clusters are in use). Allocate new one and set as directory last cluster
    return alloc_last_dir_cluster_and_alloc_entry(parent_dir, e);
}

bool VolumeFat32::alloc_first_dir_cluster_and_alloc_entry(Fat32Entry& parent_dir, Fat32Entry& e) const {
    u32 new_cluster = attach_new_directory_cluster(parent_dir);
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return false;

    // setup directory head
    parent_dir.data_cluster = new_cluster;

    // setup file entry
    e.entry_cluster = new_cluster;
    e.entry_sector = 0;
    e.entry_index = 0;
    write_entry(e);

    // setting NO_MORE marker not needed as fresh cluster has been cleared and effect is the same as NO_MORE
    return true;
}

bool VolumeFat32::alloc_last_dir_cluster_and_alloc_entry(Fat32Entry& parent_dir, Fat32Entry& e) const {
    u32 new_cluster = attach_new_directory_cluster(parent_dir);
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return false;

    // setup file entry
    e.entry_cluster = new_cluster;
    e.entry_sector = 0;
    e.entry_index = 0;
    write_entry(e);

    // setting NO_MORE marker not needed as fresh new_cluster has been cleared
    return true;
}

//-------------------------

EnumerateResult VolumeFat32::enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector, u8 start_index) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;

    for (u8 sector_offset = start_sector; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (usually 512 bytes)
        fat_data.read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = start_index; i < entries.size(); i++) { // iterate directory entries
            auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE)
                return EnumerateResult::ENUMERATION_FINISHED;    // no more entries for this dir

            if (e.name[0] == DIR_ENTRY_UNUSED)
                continue;       // unused entry, skip

            if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                continue;       // partition label

            if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                continue;   // extension for 8.3 filename

            Fat32Entry se = Fat32Entry::make_simple_dentry(fat_table, fat_data, e, cluster, sector_offset, i);
            if (!on_entry(se))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE; // continue reading the entries in next cluster
}

bool VolumeFat32::try_alloc_entry_in_free_dir_slot(const Fat32Entry& parent_dir, Fat32Entry &e) const {
    u32 cluster = parent_dir.data_cluster;
    u8 entry_type = DIR_ENTRY_NOT_FOUND;

    // try find free slot in direcyory clusters
    while (fat_table.is_allocated_cluster(cluster)) {
        entry_type = get_free_entry_in_dir_cluster(cluster, e);
        if (entry_type != DIR_ENTRY_NOT_FOUND)
            break; // free entry found

        cluster = fat_table.get_next_cluster(cluster);
        klog.format("Next Cluster: %\n", cluster);
    }

    // UNUSED entry found - just reuse it
    if (entry_type == DIR_ENTRY_UNUSED) {
        klog.format("Unused entry found. Cluster: %\n", e.entry_cluster);
        write_entry(e);
        return true;
    }

    // NO_MORE entry found - reuse it and mark the next one as NO_MORE
    if (entry_type == DIR_ENTRY_NO_MORE) {
        klog.format("NO MORE entry found. Cluster: %\n", e.entry_cluster);
        write_entry(e);
        mark_next_entry_as_nomore(e);
        return true;
    }

    return false; // no free slot found
}

/**
 * @brief   Attach new data cluster to the directory
 * @return  Cluster number if success, Fat32Table::CLUSTER_END_OF_CHAIN otherwise
 */
u32 VolumeFat32::attach_new_directory_cluster(Fat32Entry& dir) const {
    klog.format("attach_new_directory_cluster\n");

    // try alloc new cluster
    u32 new_cluster = fat_table.alloc_cluster();
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return Fat32Table::CLUSTER_END_OF_CHAIN; // new cluster allocation failed

    klog.format("Allocated new_cluster %\n", new_cluster);

    // clear data cluster
    fat_data.clear_data_cluster(new_cluster);

    // attach new_cluster to the dir as either head or last cluster
    if (dir.data_cluster == Fat32Table::CLUSTER_UNUSED)
        set_entry_data_cluster(dir, new_cluster);
    else {
        // attach data cluster to the end of chain
        u32 last_cluster = fat_table.get_last_cluster(dir.data_cluster);
        fat_table.set_next_cluster(last_cluster, new_cluster);
    }

    return new_cluster;
}

void VolumeFat32::detach_directory_cluster(const Fat32Entry& parent_dir, u32 cluster) const {
    u32 new_first_cluster = fat_table.detach_cluster(parent_dir.data_cluster, cluster);
    if (new_first_cluster != parent_dir.data_cluster)
        set_entry_data_cluster(parent_dir, new_first_cluster);
}

/**
 * @brief   Check if entry is the last entry present in parent_dir
 * @return  True if there is no valid entries after our entry. False otherwise
 */
bool VolumeFat32::is_no_more_entires_after(const Fat32Entry& parent_dir, const Fat32Entry& entry) const {
    auto on_entry = [&entry](const Fat32Entry& e) -> bool {
        static bool entry_found = false;

        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        if (entry_found)
            return false; // entry after our entry found, stop enumeration

        if (e.entry_cluster == entry.entry_cluster && e.entry_sector == entry.entry_sector && e.entry_index == entry.entry_index)
            entry_found = true; // our entry found, mark this fact

        return true;
    };

    return (enumerate_directory_entry(parent_dir, on_entry) == EnumerateResult::ENUMERATION_FINISHED); // finished means no more entries after entry in parent_dir
}

bool VolumeFat32::is_file_empty(const Fat32Entry& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}

bool VolumeFat32::is_directory_empty(const Fat32Entry& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}

/**
 * @brief   Alloc new cluster and assign it as file data cluster
 * @return  Allocated cluster
 */
u32 VolumeFat32::alloc_first_file_cluster(Fat32Entry& file) const {
    // file has no data clusters yet - alloc and assign as first data cluster
    u32 cluster = fat_table.alloc_cluster();
    file.data_cluster = cluster;
    return cluster;
}

/**
 * @brief   Alloc new cluster and attach it after "cluster"
 * @return  Allocated cluster
 */
u32 VolumeFat32::attach_next_cluster(u32 cluster) const {
    // position points to beginning of new cluster, need to alloc this new cluster
    u32 next_cluster = fat_table.alloc_cluster();
    fat_table.set_next_cluster(cluster, next_cluster);
    return next_cluster;
}

/**
 * @brief   Scan directory cluster looking for either UNUSED or NO_MORE entry
 * @param   out Free entry, if found, is stored here
 * @return  DIR_ENTRY_UNUSED, DIR_ENTRY_NO_MORE or 0xFF if not found
 */
u8 VolumeFat32::get_free_entry_in_dir_cluster(u32 cluster, Fat32Entry& out) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;

    for (u8 sector_offset = 0; sector_offset < vbr.sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (usually 512 bytes)
        fat_data.read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
            const auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE || e.name[0] == DIR_ENTRY_UNUSED) {
                out.entry_cluster = cluster;
                out.entry_sector = sector_offset;
                out.entry_index = i;
                return e.name[0];
            }
        }
    }
    return DIR_ENTRY_NOT_FOUND; // no free entry found in this cluster
}

void VolumeFat32::write_entry(const Fat32Entry& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    fat_data.read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index] = Fat32Entry::make_directory_entry_fat32(e);
    fat_data.write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void VolumeFat32::mark_entry_as_nomore(const Fat32Entry& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    fat_data.read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_NO_MORE; // mark entry as last one
    fat_data.write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void VolumeFat32::mark_next_entry_as_nomore(const Fat32Entry& e) const {
    // if e is the very last entry in the cluster - CLUSTER_END_OF_DIRECTORY will mark the end of directory entries
    if ((e.entry_sector == vbr.sectors_per_cluster - 1) && (e.entry_index == FAT32ENTRIES_PER_SECTOR - 1))
        return;

    Fat32Entry no_more = empty_entry();
    no_more.entry_cluster = e.entry_cluster;
    no_more.entry_sector = (e.entry_index < 15) ? e.entry_sector : e.entry_sector + 1;
    no_more.entry_index = (e.entry_index < 15) ? e.entry_index + 1 : 0;

    mark_entry_as_nomore(no_more);
}

void VolumeFat32::mark_entry_as_unused(const Fat32Entry& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    fat_data.read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_UNUSED; // mark entry as deleted
    fat_data.write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void VolumeFat32::set_entry_data_cluster(const Fat32Entry& e, u32 first_cluster) const {
    // update head
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    fat_data.read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].first_cluster_lo = first_cluster & 0xFFFF;
    entries[e.entry_index].first_cluster_hi = first_cluster >> 16;
    fat_data.write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

bool VolumeFat32::is_directory_cluster_empty(u32 cluster) const {
    auto on_entry = [](const Fat32Entry& e) -> bool {
        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        return false; // file found, stop enumeration
    };
    return (enumerate_directory_cluster(cluster, on_entry) != EnumerateResult::ENUMERATION_STOPPED);
}

} /* namespace filesystem */
