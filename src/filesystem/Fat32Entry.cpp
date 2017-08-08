/**
 *   @file: Fat32Entry.cpp
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Entry.h"
#include "Fat32Utils.h"

using namespace kstd;
using namespace utils;
namespace filesystem {

Fat32Entry Fat32Entry::make_simple_dentry(const Fat32Table& fat_table, const Fat32Data& fat_data, const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return Fat32Entry(fat_table, fat_data,
            ext.empty() ? name : name + "." + ext, dentry.size,
            (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
            dentry.first_cluster_hi << 16 | dentry.first_cluster_lo, entry_cluster, entry_sector, entry_index);

}

DirectoryEntryFat32 Fat32Entry::make_directory_entry_fat32(const Fat32Entry& e) {
    DirectoryEntryFat32 result;
    string name, ext;

    Fat32Utils::make_8_3_space_padded_filename(e.name, name, ext);
    memcpy(result.name, name.data(), 8);
    memcpy(result.ext, ext.data(), 3);
    result.a_time = 0;
    result.w_date = 0;
    result.w_time = 0;
    result.c_date = 0;
    result.c_time = 0;
    result.c_time_tenth = 0;
    result.attributes = e.is_directory ? DirectoryEntryFat32Attrib::DIRECTORY : 0;
    result.first_cluster_hi = e.data_cluster >> 16;
    result.first_cluster_lo = e.data_cluster & 0xFFFF;
    result.reserved = 0;
    result.size = e.size;

    return result;
}

Fat32Entry::Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data) :
        Fat32Entry(fat_table, fat_data, "", 0, false, 0, 0, 0, 0) {
}

Fat32Entry::Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data, const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 entry_cluster,
        u16 entry_sector, u8 entry_index_no) :
        fat_table(fat_table),
        fat_data(fat_data),
        name(name),
        size(size),
        is_directory(is_directory),
        data_cluster(data_cluster),
        entry_cluster(entry_cluster),
        entry_sector(entry_sector),
        entry_index(entry_index_no),
        position(0),
        position_data_cluster(data_cluster),
        klog(KernelLog::instance()) {
}

Fat32Entry& Fat32Entry::operator=(const Fat32Entry& other) {
    new (this) Fat32Entry(other);
    return *this;
}

/**
 * @brief   Read a maximum of "count" bytes, starting from file.position, into data buffer
 * @param   data  Buffer that has at least "count" capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 Fat32Entry::read(void* data, u32 count) {
    if (is_directory) {
        klog.format("Fat32Entry::read: entry is a directory\n");
        return 0;
    }

    if (position > size) {
        klog.format("Fat32Entry::read: tried reading after end of file\n");
        return 0;
    }

    // 1. setup reading status constants and variables
    const u32 MAX_BYTES_TO_READ = size - position;
    u32 total_bytes_read = 0;
    u32 remaining_bytes_to_read = min(count, MAX_BYTES_TO_READ);

    // 2. locate reading start point
    u32 position_in_cluster = position; // modulo CLUSTER_SIZE_IN_BYTES but this is done in fat_data.read_data_cluster anyway
    u32 cluster = position_data_cluster;

    // 3. follow cluster chain and read data from sectors until requested number of bytes is read
    u8* dst = (u8*) data;
    while (fat_table.is_allocated_cluster(cluster)) {
        // read the cluster until end of cluster or requested number of bytes is read
        u32 count = fat_data.read_data_cluster(position_in_cluster, cluster, dst, remaining_bytes_to_read);
        remaining_bytes_to_read -= count;
        total_bytes_read += count;
        dst += count;

        // move on to the next cluster if needed
        if (fat_data.is_cluster_beginning(position + total_bytes_read)) {
            position_in_cluster = 0;
            cluster = fat_table.get_next_cluster(cluster);
        }

        // stop reading if requested number of bytes is read
        if (remaining_bytes_to_read == 0)
            break;
    }

    // 4. done; update file position
    position += total_bytes_read;
    position_data_cluster = cluster;
    return total_bytes_read;
}

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
//u32 Fat32Entry::write(const void* data, u32 count) {
//    // 1. setup writing status variables
//    u32 total_bytes_written = 0;
//    u32 remaining_bytes_to_write = count;
//
//    // 2. locate writing start point
//    u32 position_in_cluster = position;
//    u32 cluster = get_cluster_for_write(file);
//
//    // 3. follow/make cluster chain and write data to sectors until requested number of bytes is written
//    const u8* src = (const u8*)data;
//    while (fat_table.is_allocated_cluster(cluster)) {
//        // write the cluster until end of cluster or requested number of bytes is written
//        u32 count = fat_data.write_data_cluster(position_in_cluster, cluster, src, remaining_bytes_to_write);
//        remaining_bytes_to_write -= count;
//        total_bytes_written += count;
//
//        // stop writing if requested number of bytes is written
//        if (remaining_bytes_to_write == 0)
//            break;
//
//        // move on to the next cluster
//        src += count;
//        position_in_cluster = 0;
//        cluster = attach_next_cluster(cluster);
//    }
//
//    // 4. done; update file position and size if needed
//    position += total_bytes_written;
//    position_data_cluster = cluster;
//
//    if (size < position) {
//        size = position;
//        write_entry(file);
//    }
//
//    return total_bytes_written;
//}

/**
 * @brief   Move file current position to given "position" if possible
 */
void Fat32Entry::seek(u32 new_position) {
    if (entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("Fat32Entry::seek: uninitialized entry\n");
        return;
    }

    if (is_directory){
        klog.format("Fat32Entry::seek: entry is a directory\n");
        return;
    }

    if (new_position > size) {
        klog.format("Fat32Entry::seek: new_position > size (% > %)\n", new_position, size);
        return;
    }

    position_data_cluster = fat_table.find_cluster_for_byte(data_cluster, new_position);
    position = new_position;
}


Fat32Entry::operator bool() const {
    return !name.empty();
}

bool Fat32Entry::operator!() const {
    return name.empty();
}

} /* namespace filesystem */
