/**
 *   @file: Fat32Entry.cpp
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "Fat32Entry.h"
#include "Fat32Utils.h"

using std::array;
using namespace kstd;
using namespace utils;
namespace filesystem {


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
    result.first_cluster_hi = e.data.get_head() >> 16;
    result.first_cluster_lo = e.data.get_head() & 0xFFFF;
    result.reserved = 0;
    result.size = e.data.get_size();

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
        is_directory(is_directory),
        data(fat_table, fat_data, data_cluster, size),
        entry_cluster(entry_cluster),
        entry_sector(entry_sector),
        entry_index(entry_index_no),
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
    if (entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("Fat32Entry::read: uninitialized entry\n");
        return 0;
    }

    if (is_directory) {
        klog.format("Fat32Entry::read: entry is a directory\n");
        return 0;
    }

    return this->data.read(data, count);
}

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 Fat32Entry::write(const void* data, u32 count) {
    if (entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("Fat32Entry::write: uninitialized entry\n");
        return 0;
    }

    if (is_directory) {
        klog.format("Fat32Entry::write: specified entry is a directory\n");
        return 0;
    }

    if ((u64)this->data.get_size() + count > 0xFFFFFFFF){
        klog.format("Fat32Entry::write: would exceed Fat32 4GB limit\n");
        return 0;
    }

    u32 old_size = this->data.get_size();
    u32 total_bytes_written = this->data.write(data, count);
    if (old_size != this->data.get_size())
        write_entry();

    return total_bytes_written;
}

/**
 * @brief   Move file current position to given "position" if possible
 */
void Fat32Entry::seek(u32 new_position) {
    if (data.empty()) {
        klog.format("Fat32Entry::seek: uninitialized entry\n");
        return;
    }

    if (is_directory){
        klog.format("Fat32Entry::seek: entry is a directory\n");
        return;
    }

    if (new_position > data.get_size()) {
        klog.format("Fat32Entry::seek: new_position > size (% > %)\n", new_position, data.get_size());
        return;
    }

    data.seek(new_position);
}

/**
 * @brief   Resize file, if new_size > current size, extra space is overwritten with zeroes
 */
void Fat32Entry::truncate(u32 new_size) {

    if (entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("Fat32Entry::truncate: uninitialized entry\n");
        return;
    }

    if (is_directory){
        klog.format("Fat32Entry::truncate: entry is a directory\n");
        return;
    }

    if (new_size > data.get_size()) {
         // move to the old file end
         u32 old_position = data.get_position();
         data.seek(data.get_size());

         // calc number of zeroes needed
         u32 remaining_zeroes = new_size - data.get_size();

         // prepare zeroes
         const u16 SIZE = 512;
         u8 zeroes[SIZE];
         memset(zeroes, 0, SIZE);

         // fill file tail with zeroes
         while (remaining_zeroes != 0) {
             u32 count = min(remaining_zeroes, SIZE);
             klog.format("writing % zeroes \n", count);
             write(zeroes, count);
             remaining_zeroes -= count;
         }
         data.seek(old_position);
         // file.size already updated by write
         // dont change file position
    } else
    if (new_size < data.get_size()) {
        data.resize(new_size);
    }

    write_entry();

}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
EnumerateResult Fat32Entry::enumerate_entries(const OnEntryFound& on_entry) const {
    if (entry_cluster == Fat32Table::CLUSTER_UNUSED) {
        klog.format("Fat32Entry::enumerate_entries: uninitialized entry\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    if (!is_directory){
        klog.format("Fat32Entry::enumerate_entries: not a directory\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    u32 cluster = data.get_head();

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

Fat32Entry::operator bool() const {
    return !name.empty();
}

bool Fat32Entry::operator!() const {
    return name.empty();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void Fat32Entry::write_entry() const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    fat_data.read_data_sector(entry_cluster, entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[entry_index] = Fat32Entry::make_directory_entry_fat32(*this);
    fat_data.write_data_sector(entry_cluster, entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

/**
 * @brief   Enumerate entries in single directory cluster, starting from  [start_sector][start_index]
 */
EnumerateResult Fat32Entry::enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector, u8 start_index) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;

    for (u8 sector_offset = start_sector; sector_offset < fat_data.get_sectors_per_cluster(); sector_offset++) { // iterate sectors in cluster

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

            Fat32Entry se = make_simple_dentry(e, cluster, sector_offset, i);
            if (!on_entry(se))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE; // continue reading the entries in next cluster
}

Fat32Entry Fat32Entry::make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) const {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return Fat32Entry(fat_table, fat_data,
            ext.empty() ? name : name + "." + ext, dentry.size,
            (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
            dentry.first_cluster_hi << 16 | dentry.first_cluster_lo, entry_cluster, entry_sector, entry_index);

}

} /* namespace filesystem */
