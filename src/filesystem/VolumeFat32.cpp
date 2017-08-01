/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */


#include <algorithm>
#include "KernelLog.h"
#include "VolumeFat32.h"
#include "Fat32Utils.h"
#include "kstd.h"

using namespace kstd;
using namespace utils;
namespace filesystem {

/**
 * Constructor.
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
 * @brief   Read a maximum of "count" bytes, starting from file.position, into data buffer
 * @param   file  File entry to read from
 * @param   data  Buffer that has at least "count" capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::read_file_entry(SimpleDentryFat32& file, void* data, u32 count) const {
    if (file.is_directory)  // please dont read raw data from directory entry
        return 0;

    if (file.position >= file.size) // please dont read after End Of File
        return 0;

    // 1. setup reading status variables
    const u16 SECTOR_SIZE_IN_BYTES = vbr.bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = vbr.sectors_per_cluster * SECTOR_SIZE_IN_BYTES;
    const u32 MAX_BYTES_TO_READ = file.size - file.position;
    u32 total_bytes_read = 0;
    u32 remaining_bytes_to_read = (count < MAX_BYTES_TO_READ) ? count : MAX_BYTES_TO_READ;

    // 2. locate reading start point
    u32 cluster = fat_table.get_or_alloc_cluster_for_byte(file.data_cluster, file.position);    // get already allocated data cluster
    u8 sector_in_cluster = (file.position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u16 byte_in_sector = (file.position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;

    // 3. follow cluster chain and read data from sectors until requested number of bytes is read
    u8* dst = (u8*)data;
    while (fat_table.is_allocated_cluster(cluster)) {
        for (; sector_in_cluster < vbr.sectors_per_cluster; sector_in_cluster++) {
            u16 bytes_in_sector_left = SECTOR_SIZE_IN_BYTES - byte_in_sector;
            u16 read_count = remaining_bytes_to_read < bytes_in_sector_left ? remaining_bytes_to_read : bytes_in_sector_left;
            fat_data.read_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, dst, read_count);
            remaining_bytes_to_read -= read_count;
            total_bytes_read += read_count;
            dst += read_count;
            byte_in_sector = 0;
            if (remaining_bytes_to_read == 0)
                break;
        }

        if (remaining_bytes_to_read == 0)
            break;

        cluster = fat_table.get_next_cluster(cluster);
        sector_in_cluster = 0;
    }

    // 4. done
    file.position += total_bytes_read;
    return total_bytes_read;
}

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   file File entry to write to
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 VolumeFat32::write_file_entry(SimpleDentryFat32& file, void const* data, u32 count) const {
    if (file.is_directory)  // please dont write raw data into directory entry
        return 0;

    if (file.position + count > 0xFFFFFFFF) // FAT32 4GB size limit
        return 0;

    // 1. setup writing status variables
    const u16 SECTOR_SIZE_IN_BYTES = vbr.bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = vbr.sectors_per_cluster * SECTOR_SIZE_IN_BYTES;
    u32 total_bytes_written = 0;
    u32 remaining_bytes_to_write = count;

    // 2. locate writing start point
    u32 cluster = fat_table.get_or_alloc_cluster_for_byte(file.data_cluster, file.position);
    file.data_cluster = file.data_cluster == Fat32Table::CLUSTER_UNUSED ? cluster : file.data_cluster;
    u8 sector_in_cluster = (file.position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u16 byte_in_sector = (file.position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;

    // 3. follow/make cluster chain and write data to sectors until requested number of bytes is written
    u8 const* src = (u8 const*)data;
    while (fat_table.is_allocated_cluster(cluster)) {
        for (; sector_in_cluster < vbr.sectors_per_cluster; sector_in_cluster++) {
            u16 bytes_in_sector_left = SECTOR_SIZE_IN_BYTES - byte_in_sector;
            u16 written_count = remaining_bytes_to_write < bytes_in_sector_left ? remaining_bytes_to_write : bytes_in_sector_left;
            fat_data.write_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, src, written_count);
            remaining_bytes_to_write -= written_count;
            total_bytes_written += written_count;
            src += written_count;
            byte_in_sector = 0;
            if (remaining_bytes_to_write == 0)
                break;
        }

        if (remaining_bytes_to_write == 0)
            break;

        u32 prev_cluster = cluster;
        cluster = fat_table.alloc_cluster();
        fat_table.set_next_cluster(prev_cluster, cluster);
        sector_in_cluster = 0;
    }

    // 4. done
    file.position += total_bytes_written;
    file.size = file.size > file.position ? file.size : file.position;
    fat_data.write_entry(file);
    return total_bytes_written;
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
bool VolumeFat32::create_entry(const kstd::string& unix_path, bool directory) const {
    if (unix_path.empty() || unix_path == "/")
        return false;

    // check if parent_dir exists
    SimpleDentryFat32 parent_dir;
    if (!get_entry(extract_file_directory(unix_path), parent_dir)) {
        klog.format("Parent not exists: %\n", extract_file_directory(unix_path));
        return false;
    }

    // check if entry already exists
    string name = extract_file_name(unix_path);
    string name_8_3 = Fat32Utils::make_8_3_filename(name);
    SimpleDentryFat32 tmp;
    if (get_entry_for_name(parent_dir, name_8_3, tmp)) {
        klog.format("Entry already exists: %(%)\n", name_8_3, unix_path);
        return false;   // entry exists
    }

    // allocate entry in parent_dir
    SimpleDentryFat32 e;
    e.name = name;
    e.is_directory = directory;
    klog.format("Creating entry. Name: %\n", e.name);
    return alloc_entry_in_directory(parent_dir, e);
}

/**
 * @brief   Delete file or empty directory
 * @return  True on successful deletion, False if:
 *          -empty path specified
 *          -root dir specified
 *          -path does not exist
 *          -path points to a dir that is not empty
 */
bool VolumeFat32::delete_entry(const string& unix_path) const {
    if (unix_path.empty() || unix_path == "/")
        return false;

    // get entry parent dir to be updated
    SimpleDentryFat32 parent_dir;
    if (!get_entry(extract_file_directory(unix_path), parent_dir))
        return false;

    // get entry to be deleted
    SimpleDentryFat32 e;
    if (!get_entry_for_name(parent_dir, extract_file_name(unix_path), e))
        return false;

    // for directory - ensure it is empty
    if (e.is_directory && !is_directory_empty(e))
        return false;

    // for file - release its data
    if (!e.is_directory)
        fat_table.free_cluster_chain(e.data_cluster);

    // mark entry as nomore/unused depending on it's position in the directory
    if (is_no_more_entires_after(parent_dir, e))
        fat_data.mark_entry_as_nomore(e);
    else
        fat_data.mark_entry_as_unused(e);

    // if cluster where our entry was allocated contains no more files - remove it from the chain.
    // but dont remove root first cluster!
    if (e.entry_cluster != vbr.root_cluster && fat_data.is_directory_cluster_empty(e.entry_cluster))
        detach_directory_cluster(parent_dir, e.entry_cluster);

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
bool VolumeFat32::get_entry_for_name(const SimpleDentryFat32& parent_dir, const string& filename, SimpleDentryFat32& out_entry) const {
    auto on_entry = [&filename, &out_entry](const SimpleDentryFat32& e) -> bool {
        if (e.name == filename) {
            out_entry = e;
            return false;   // entry found. stop enumeration
        } else
            return true;    // continue searching for entry
    };

    return enumerate_directory_entry(parent_dir, on_entry) == EnumerateResult::ENUMERATION_STOPPED; // enumeration stopped means entry found
}

/**
 * @brief   Find empty slot in parent dir or attach new data cluster and allocate entry in it.
 *          parent_dir.data_cluster can be modified if first cluster is allocated for this directory
 *          entry cluster, segment and index are set to describe the allocated position in parent_dir
 * @return  True if entry was succesfully allocated in parent_di
 */
bool VolumeFat32::alloc_entry_in_directory(SimpleDentryFat32& parent_dir, SimpleDentryFat32 &e) const {
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

bool VolumeFat32::alloc_first_dir_cluster_and_alloc_entry(SimpleDentryFat32& parent_dir, SimpleDentryFat32& e) const {
    u32 new_cluster = attach_new_directory_cluster(parent_dir);
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return false;

    // setup directory head
    parent_dir.data_cluster = new_cluster;

    // setup file entry
    e.entry_cluster = new_cluster;
    e.entry_sector = 0;
    e.entry_index = 0;
    fat_data.write_entry(e);

    // setting NO_MORE marker not needed as fresh cluster has been cleared and effect is the same as NO_MORE
    return true;
}

bool VolumeFat32::alloc_last_dir_cluster_and_alloc_entry(SimpleDentryFat32& parent_dir, SimpleDentryFat32& e) const {
    u32 new_cluster = attach_new_directory_cluster(parent_dir);
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return false;

    // setup file entry
    e.entry_cluster = new_cluster;
    e.entry_sector = 0;
    e.entry_index = 0;
    fat_data.write_entry(e);

    // setting NO_MORE marker not needed as fresh new_cluster has been cleared
    return true;
}

bool VolumeFat32::try_alloc_entry_in_free_dir_slot(const SimpleDentryFat32& parent_dir, SimpleDentryFat32 &e) const {
    u32 cluster = parent_dir.data_cluster;
    u8 entry_type = Fat32Data::DIR_ENTRY_NOT_FOUND;

    // try find free slot in direcyory clusters
    while (fat_table.is_allocated_cluster(cluster)) {
        entry_type = fat_data.get_free_entry_in_dir_cluster(cluster, e);
        if (entry_type != Fat32Data::DIR_ENTRY_NOT_FOUND)
            break; // free entry found

        cluster = fat_table.get_next_cluster(cluster);
        klog.format("Next Cluster: %\n", cluster);
    }

    // UNUSED entry found - just reuse it
    if (entry_type == Fat32Data::DIR_ENTRY_UNUSED) {
        klog.format("Unused entry found. Cluster: %\n", e.entry_cluster);
        fat_data.write_entry(e);
        return true;
    }

    // NO_MORE entry found - reuse it and mark the next one as NO_MORE
    if (entry_type == Fat32Data::DIR_ENTRY_NO_MORE) {
        klog.format("NO MORE entry found. Cluster: %\n", e.entry_cluster);
        fat_data.write_entry(e);
        fat_data.mark_next_entry_as_nomore(e);
        return true;
    }

    return false; // no free slot found
}

/**
 * @brief   Attach new data cluster to the directory
 * @return  Cluster number if success, Fat32Table::CLUSTER_END_OF_CHAIN otherwise
 */
u32 VolumeFat32::attach_new_directory_cluster(SimpleDentryFat32& dir) const {
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
        fat_data.set_entry_data_cluster(dir, new_cluster);
    else {
        // attach data cluster to the end of chain
        u32 last_cluster = fat_table.get_last_cluster(dir.data_cluster);
        fat_table.set_next_cluster(last_cluster, new_cluster);
    }

    return new_cluster;
}

void VolumeFat32::detach_directory_cluster(const SimpleDentryFat32& parent_dir, u32 cluster) const {
    u32 new_first_cluster = fat_table.detach_cluster(parent_dir.data_cluster, cluster);
    if (new_first_cluster != parent_dir.data_cluster)
        fat_data.set_entry_data_cluster(parent_dir, new_first_cluster);
}

/**
 * @brief   Check if entry is the last entry present in parent_dir
 * @return  True if there is no valid entries after our entry. False otherwise
 */
bool VolumeFat32::is_no_more_entires_after(const SimpleDentryFat32& parent_dir, const SimpleDentryFat32& entry) const {
    auto on_entry = [&entry](const SimpleDentryFat32& e) -> bool {
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

bool VolumeFat32::is_directory_empty(const SimpleDentryFat32& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}
} /* namespace filesystem */
