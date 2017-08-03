/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */


#include "VolumeFat32.h"
#include "Fat32Utils.h"

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
 * @return  True if entry exists, False otherwise
 */
bool VolumeFat32::get_entry(const string& unix_path, SimpleDentryFat32& out_entry) const {
    if (unix_path.empty() || unix_path.front() != '/')
        return false;

    // start at root...
    SimpleDentryFat32 e = get_root_dentry();

    // ...and descend down the path to the very last entry
    auto normalized_unix_path = Fat32Utils::normalize_path(unix_path); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e.is_directory) {
            klog.format("VolumeFat32::get_entry: entry '%' is not a directory\n", e.name);
            return false;   // path segment is not a directory. this is error
        }

        if (!get_entry_for_name(e, path_segment, e)) {
            klog.format("VolumeFat32::get_entry: entry '%' does not exist\n", path_segment);
            return false;   // path segment does not exist. this is error
        }
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
    if (file.entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("VolumeFat32::read_file_entry: uninitialized file entry to read specified\n");
        return 0;
    }

    if (file.is_directory) {
        klog.format("VolumeFat32::read_file_entry: specified entry is a directory\n");
        return 0;
    }

    if (file.position > file.size) {
        klog.format("VolumeFat32::read_file_entry: tried reading after end of file\n");
        return 0;
    }

    return read_file_data(file, data, count);
}

/**
 * @brief   Read a maximum of "count" bytes, starting from file.position, into data buffer
 * @param   file  File entry to read from
 * @param   data  Buffer that has at least "count" capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::read_file_data(SimpleDentryFat32& file, void* data, u32 count) const {
    // 1. setup reading status constants and variables
    const u16 SECTOR_SIZE_IN_BYTES = vbr.bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = vbr.bytes_per_sector * vbr.sectors_per_cluster;
    const u32 MAX_BYTES_TO_READ = file.size - file.position;
    u32 total_bytes_read = 0;
    u32 remaining_bytes_to_read = (count < MAX_BYTES_TO_READ) ? count : MAX_BYTES_TO_READ;
    
    // 2. locate reading start point
    u16 byte_in_sector = (file.position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;
    u8 sector_in_cluster = (file.position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u32 cluster = file.position_data_cluster;
    
    // 3. follow cluster chain and read data from sectors until requested number of bytes is read
    u8* dst = (u8*) (data);
    while (fat_table.is_allocated_cluster(cluster)) {
        // read the cluster until end of cluster or requested number of bytes is read
        u32 count = read_cluster_data(byte_in_sector, sector_in_cluster, cluster, dst, remaining_bytes_to_read);
        remaining_bytes_to_read -= count;
        total_bytes_read += count;
        dst += count;

        // move on to the next cluster if needed
        if (is_next_cluster_needed(file.position + total_bytes_read)) {
            byte_in_sector = 0;
            sector_in_cluster = 0;
            cluster = fat_table.get_next_cluster(cluster);
        }

        // stop reading if requested number of bytes is read
        if (remaining_bytes_to_read == 0)
            break;
    }
    
    // 4. done; update file position
    file.position += total_bytes_read;
    file.position_data_cluster = cluster;
    return total_bytes_read;
}

/**
 * @brief   Read the cluster starting from [cluster][sector_in_cluster][byte_in_sector] until "count" bytes is read or end of cluster is reached
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::read_cluster_data(u16 byte_in_sector, u8 sector_in_cluster, u32 cluster, u8* data, u32 count) const {
    const u16 SECTOR_SIZE_IN_BYTES = vbr.bytes_per_sector;
    u32 total_bytes_read = 0;

    for (; sector_in_cluster < vbr.sectors_per_cluster; sector_in_cluster++) {
        u16 bytes_in_sector_left = SECTOR_SIZE_IN_BYTES - byte_in_sector;
        u16 read_count = count < bytes_in_sector_left ? count : bytes_in_sector_left;

        if (!fat_data.read_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, data, read_count))
            break;

        count -= read_count;
        total_bytes_read += read_count;
        data += read_count;
        byte_in_sector = 0;

        if (count == 0)
            break;
    }
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
u32 VolumeFat32::write_file_data(SimpleDentryFat32& file, const void* data, u32 count) const {
    // 1. setup writing status variables
    const u16 SECTOR_SIZE_IN_BYTES = vbr.bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = vbr.sectors_per_cluster * SECTOR_SIZE_IN_BYTES;
    u32 total_bytes_written = 0;
    u32 remaining_bytes_to_write = count;

    // 2. locate writing start point
    u16 byte_in_sector = (file.position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;
    u8 sector_in_cluster = (file.position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u32 cluster = get_cluster_for_write(file);

    // 3. follow/make cluster chain and write data to sectors until requested number of bytes is written
    const u8* src = (const u8*)data;
    while (fat_table.is_allocated_cluster(cluster)) {
        // write the cluster until end of cluster or requested number of bytes is written
        u32 count = write_cluster_data(byte_in_sector, sector_in_cluster, cluster, src, remaining_bytes_to_write);
        remaining_bytes_to_write -= count;
        total_bytes_written += count;

        // stop writing if requested number of bytes is written
        if (remaining_bytes_to_write == 0)
            break;

        // move on to the next cluster
        src += count;
        byte_in_sector = 0;
        sector_in_cluster = 0;
        cluster = alloc_next_file_cluster(cluster);
    }

    // 4. done; update file position and size if needed
    file.position += total_bytes_written;
    file.position_data_cluster = cluster;

    if (file.size < file.position) {
        file.size = file.position;
        fat_data.write_entry(file);
    }

    return total_bytes_written;
}

/**
 * @brief   Get proper cluster for data writing depending on file status and file position
 * @return  Cluster for writing data
 */
u32 VolumeFat32::get_cluster_for_write(SimpleDentryFat32& file) const {
    u32 cluster;
    if (is_file_empty(file)) {
        // file has no data clusters yet - alloc and assign as first data cluster
        cluster = fat_table.alloc_cluster();
        file.data_cluster = cluster;
    } else if (is_next_cluster_needed(file.position)) {
        // position points to beginning of new cluster - alloc this new cluster
        u32 last_cluster = fat_table.get_last_cluster(file.data_cluster);
        cluster = fat_table.alloc_cluster();
        fat_table.set_next_cluster(last_cluster, cluster);
    } else {
        cluster = file.position_data_cluster; // just use the current cluster as it has space in it
    }

    return cluster;
}

/**
 * @brief   Write the cluster starting from [cluster][sector_in_cluster][byte_in_sector] until "count" bytes is read or end of cluster is reached
 * @return  Number of bytes actually read
 */
u32 VolumeFat32::write_cluster_data(u16 byte_in_sector, u8 sector_in_cluster, u32 cluster, const u8* data, u32 count) const {
    u32 total_bytes_written = 0;

    for (; sector_in_cluster < vbr.sectors_per_cluster; sector_in_cluster++) {
        u16 bytes_in_sector_left = vbr.bytes_per_sector - byte_in_sector;
        u16 written_count = count < bytes_in_sector_left ? count : bytes_in_sector_left;

        if (!fat_data.write_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, data, written_count))
            break;

        count -= written_count;
        total_bytes_written += written_count;
        data += written_count;
        byte_in_sector = 0;

        if (count == 0)
            break;
    }
    return total_bytes_written;
}

/**
 * @brief   Check if we are at the beginning of a brand new cluster
 */
bool VolumeFat32::is_next_cluster_needed(u32 position) const {
    const u16 CLUSTER_SIZE_IN_BYTES = vbr.sectors_per_cluster * vbr.bytes_per_sector;
    return (position % CLUSTER_SIZE_IN_BYTES) == 0;
}

/**
 * @brief   Move file current position to given "position" if possible
 */
void VolumeFat32::seek_file_entry(SimpleDentryFat32& file, u32 position) const {
    if (file.entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("VolumeFat32::seek_file_entry: uninitialized entry specified\n");
        return;
    }

    if (file.is_directory){
        klog.format("VolumeFat32::seek_file_entry: specified entry is a directory\n");
        return;
    }

    if (position > file.size) {
        klog.format("VolumeFat32::seek_file_entry: position > size (% > %)\n", position, file.size);
        return;
    }

    file.position_data_cluster = fat_table.find_cluster_for_byte(file.data_cluster, position);
    file.position = position;
}

void VolumeFat32::trunc_file_entry(SimpleDentryFat32& file, u32 new_size) const {
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
         seek_file_entry(file, file.size);
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
             u32 count = remaining_zeroes < SIZE ? remaining_zeroes : SIZE;
             klog.format("writing % zeroes \n", count);
             write_file_entry(file, zeroes, count);
             remaining_zeroes -= count;
         }
         seek_file_entry(file, old_position);
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

    fat_data.write_entry(file);
}

/**
 * @brief   Enumerate directory contents
 * @param   dentry Directory entry for which we want to enumerate elements
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
EnumerateResult VolumeFat32::enumerate_directory_entry(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry) const {
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
    SimpleDentryFat32 tmp;
    return create_entry(unix_path, directory, tmp);
}

bool VolumeFat32::create_entry(const kstd::string& unix_path, bool directory, SimpleDentryFat32& out) const {
    if (unix_path.empty() || unix_path == "/")
        return false;

    // check if parent_dir exists
    SimpleDentryFat32 parent_dir;
    if (!get_entry(extract_file_directory(unix_path), parent_dir)) {
        klog.format("VolumeFat32::create_entry: Parent not exists: %\n", extract_file_directory(unix_path));
        return false;
    }

    // check if entry already exists
    string name = extract_file_name(unix_path);
    if (name.empty()) {
        klog.format("VolumeFat32::create_entry: Please specify name for entry to create: %\n", unix_path);
        return false;
    }

    string name_8_3 = Fat32Utils::make_8_3_filename(name);
    SimpleDentryFat32 tmp;
    if (get_entry_for_name(parent_dir, name_8_3, tmp)) {
        klog.format("VolumeFat32::create_entry: Entry already exists: %(%)\n", name_8_3, unix_path);
        return false;   // entry exists
    }

    // allocate entry in parent_dir
    out.name = name;
    out.is_directory = directory;
    return alloc_entry_in_directory(parent_dir, out);
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
    if (!get_entry(extract_file_directory(unix_path), parent_dir)) {
        klog.format("VolumeFat32::delete_entry: path doesn't exist: %\n", unix_path);
        return false;
    }

    // get entry to be deleted
    SimpleDentryFat32 e;
    if (!get_entry_for_name(parent_dir, extract_file_name(unix_path), e)) {
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
 * @brief   Move file/directory within volume
 * @param   unix_path_from Exact source entry path
 * @param   unix_path_to Exact destination path/destination directory
 * @return  True on success, False otherwise
 */
bool VolumeFat32::move_entry(const kstd::string& unix_path_from, const kstd::string& unix_path_to) const {
    // get source entry
    SimpleDentryFat32 src;
    if (!get_entry(unix_path_from, src)) {
        klog.format("VolumeFat32::move_entry: src entry doesn't exists '%'\n", unix_path_from);
        return false;
    }

    // if destination directory specified instead of full path - keep source name
    SimpleDentryFat32 dst;
    string path_to = unix_path_to;
    if (get_entry(unix_path_to, dst))
        if (dst.is_directory)
            path_to += string("/") + extract_file_name(unix_path_from);

    // create destination entry
    if (!create_entry(path_to, src.is_directory, dst)) {
        klog.format("VolumeFat32::move_entry: can't create dst entry '%'\n", path_to);
        return false;
    }

    // move data cluster chain from src path_to dst entry
    dst.data_cluster = src.data_cluster;
    src.data_cluster = Fat32Table::CLUSTER_UNUSED;
    dst.size = src.size;
    src.size = 0;
    fat_data.write_entry(src);
    fat_data.write_entry(dst);

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
SimpleDentryFat32 VolumeFat32::get_root_dentry() const {
    return SimpleDentryFat32("/", 0, true, vbr.root_cluster, Fat32Table::CLUSTER_END_OF_CHAIN, 0, 0);
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
    //fat_data.clear_data_cluster(new_cluster);

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

bool VolumeFat32::is_file_empty(const SimpleDentryFat32& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}

bool VolumeFat32::is_directory_empty(const SimpleDentryFat32& e) const {
    return e.data_cluster == Fat32Table::CLUSTER_UNUSED;
}

/**
 * @brief   Alloc new cluster and assign it as file data cluster
 * @return  Allocated cluster
 */
u32 VolumeFat32::alloc_first_file_cluster(SimpleDentryFat32& file) const {
    // file has no data clusters yet - alloc and assign as first data cluster
    u32 cluster = fat_table.alloc_cluster();
    file.data_cluster = cluster;
    return cluster;
}

/**
 * @brief   Alloc new cluster and attach it after "cluster"
 * @return  Allocated cluster
 */
u32 VolumeFat32::alloc_next_file_cluster(u32 cluster) const {
    // position points to beginning of new cluster, need to alloc this new cluster
    u32 next_cluster = fat_table.alloc_cluster();
    fat_table.set_next_cluster(cluster, next_cluster);
    return next_cluster;
}
} /* namespace filesystem */
