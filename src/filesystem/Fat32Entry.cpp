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
    result.attributes = e.is_dir ? DirectoryEntryFat32Attrib::DIRECTORY : 0;
    result.first_cluster_hi = e.data.get_head() >> 16;
    result.first_cluster_lo = e.data.get_head() & 0xFFFF;
    result.reserved = 0;
    result.size = e.is_dir ? 0 : e.data.get_size();

    return result;
}

Fat32Entry::Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data) :
        Fat32Entry(fat_table, fat_data, "", 0, false, 0, 0, 0) {
}

Fat32Entry::Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data, const kstd::string& name,
        u32 size, bool is_directory, u32 data_cluster, u32 parent_data_cluster, u32 parent_index) :

        fat_table(fat_table),
        fat_data(fat_data),
        name(name),
        is_dir(is_directory),
        data(fat_table, fat_data, data_cluster, is_directory ?  0xFFFFFFFF : size),
        parent_data(fat_table, fat_data, parent_data_cluster, 0xFFFFFFFF), // parent_data is directory data cluster which size is unknown
        parent_index(parent_index),
        klog(KernelLog::instance()) {
}

Fat32Entry& Fat32Entry::operator=(const Fat32Entry& other) {
    new (this) Fat32Entry(other);
    return *this;
}

Fat32Entry::operator bool() const {
    return !name.empty();
}

bool Fat32Entry::operator!() const {
    return name.empty();
}

bool Fat32Entry::is_directory() const {
    return is_dir;
}

u32 Fat32Entry::get_size() const {
    return data.get_size();
}

const string& Fat32Entry::get_name() const {
    return name;
}

/**
 * @brief   Read a maximum of "count" bytes, starting from current position, into data buffer
 * @param   data  Buffer that has at least "count" capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 Fat32Entry::read(void* data, u32 count) {
    if (!is_initialized()) {
        klog.format("Fat32Entry::read: uninitialized entry\n");
        return 0;
    }

    if (is_dir) {
        klog.format("Fat32Entry::read: entry is a directory\n");
        return 0;
    }

    return this->data.read(data, count);
}

/**
 * @brief   Write "count" bytes into the file, starting from current position, enlarging the file size if needed
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 Fat32Entry::write(const void* data, u32 count) {
    if (!is_initialized()) {
        klog.format("Fat32Entry::write: uninitialized entry\n");
        return 0;
    }

    if (is_dir) {
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
        update_entry_info_in_parent_dir();

    return total_bytes_written;
}

/**
 * @brief   Move file current position to given "position" if possible
 */
void Fat32Entry::seek(u32 new_position) {
    if (!is_initialized()) {
        klog.format("Fat32Entry::seek: uninitialized entry\n");
        return;
    }

    if (is_dir){
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
    if (new_size == data.get_size())
        return;

    if (!is_initialized()) {
        klog.format("Fat32Entry::truncate: uninitialized entry\n");
        return;
    }

    if (is_dir){
        klog.format("Fat32Entry::truncate: entry is a directory\n");
        return;
    }

    // enlarging the file, write zeroes
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
             write(zeroes, count);
             remaining_zeroes -= count;
         }
         // file.size already updated by write functions

         // restore old position
         data.seek(old_position);
    }
    // shrinking the file, just resize the cluster chain to new size
    else
        data.resize(new_size);

    update_entry_info_in_parent_dir();
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
EnumerateResult Fat32Entry::enumerate_entries(const OnEntryFound& on_entry) {
    if (!is_initialized()) {
        klog.format("Fat32Entry::enumerate_entries: uninitialized entry\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    if (!is_dir){
        klog.format("Fat32Entry::enumerate_entries: not a directory\n");
        return EnumerateResult::ENUMERATION_FINISHED;
    }

    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    u32 entries_size_in_bytes = FAT32ENTRIES_PER_SECTOR * sizeof(DirectoryEntryFat32);
    data.seek(0);
    u32 entry_index = 0;
    while (data.read(&entries, entries_size_in_bytes) > 0) {
        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
            const auto& e = entries[i];

            if (e.is_nomore()) {    // no more entries for this dir
                return EnumerateResult::ENUMERATION_FINISHED;
            }

            if (e.is_unused()) {    // unused entry, skip
                entry_index++;
                continue;
            }

            if (e.is_volume_id()) { // partition label, skip
                entry_index++;
                continue;
            }

            if (e.is_long_name()) { // extension for 8.3 filename, skip
                entry_index++;
                continue;
            }

            Fat32Entry se = make_simple_dentry(e, data, entry_index);
            if (!on_entry(se)) {
                return EnumerateResult::ENUMERATION_STOPPED;
            }

            entry_index++;
        }
    }
    return EnumerateResult::ENUMERATION_FINISHED; // all entries enumerated
}

/**
 * @brief   Update entry meta data in parent directory, eg it's name, size, attributes
 */
void Fat32Entry::update_entry_info_in_parent_dir() {
    DirectoryEntryFat32 dentry = make_directory_entry_fat32(*this);
    u32 position_in_parent = parent_index * sizeof(DirectoryEntryFat32);
    parent_data.seek(position_in_parent);
    u32 written = parent_data.write(&dentry, sizeof(dentry));
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
            if (e.is_nomore())
                return EnumerateResult::ENUMERATION_FINISHED;    // no more entries for this dir

            if (e.is_unused())
                continue;       // unused entry, skip

            if (e.is_volume_id())
                continue;       // partition label

            if (e.is_long_name())
                continue;   // extension for 8.3 filename

            Fat32Entry se = make_simple_dentry(e, data, 0);
            if (!on_entry(se))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE; // continue reading the entries in next cluster
}

Fat32Entry Fat32Entry::make_simple_dentry(const DirectoryEntryFat32& dentry, Fat32ClusterChain parent_data, u32 parent_index) const {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));
    bool is_directory = (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY;
    u32 data_cluster = dentry.first_cluster_hi << 16 | dentry.first_cluster_lo;

    return Fat32Entry(
            fat_table,
            fat_data,
            ext.empty() ? name : name + "." + ext,
            dentry.size,
            is_directory,
            data_cluster,
            parent_data.get_head(),
            parent_index);
}

/**
 * @brief   Find empty slot in parent dir or attach new data cluster and allocate entry in it.
 *          parent_dir.data.head can be modified if first cluster is to be allocated for this directory
 *          entry cluster, segment and index are set to describe the allocated position in parent_dir
 * @return  True if entry was successfully allocated in parent_dir
 */
bool Fat32Entry::alloc_entry_in_directory(Fat32Entry& out) {
    // first lets see if we can find a free slot for our entry
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    u32 entries_size_in_bytes = FAT32ENTRIES_PER_SECTOR * sizeof(DirectoryEntryFat32);
    u32 entry_index = 0;
    data.seek(0);

    while (data.read(&entries, entries_size_in_bytes) > 0) {
        for (const auto& e : entries) { // iterate directory entries
            if (e.is_unused())
                return alloc_entry_in_directory_at_index(entry_index, out);

            if (e.is_nomore()) {
                if (!alloc_entry_in_directory_at_index(entry_index, out))
                    return false;

                mark_next_entry_as_nomore(out); // move NO_MORE marker to the next entry
                return true;
            }

            entry_index++;
        }
    }

    // no free slot found, lets alloc new cluster and alloc our entry in it
    u32 old_head = data.get_head();
    if (!alloc_entry_in_directory_at_index(entry_index, out))
        return false;

    mark_next_entry_as_nomore(out); // need to mark next entry as NO_MORE as there can be garbage in newly allocated cluster

    if (old_head != data.get_head())
        update_entry_info_in_parent_dir();

    return true;
}

/**
 * @brief   Allocate entry at given index in directory
 */
bool Fat32Entry::alloc_entry_in_directory_at_index(u32 index_in_dir, Fat32Entry& out) {
    out.parent_data = data;
    out.parent_index = index_in_dir;
    u32 position_in_parent = index_in_dir * sizeof(DirectoryEntryFat32);
    data.seek(position_in_parent);
    DirectoryEntryFat32 dentry = make_directory_entry_fat32(out);
    return data.write(&dentry, sizeof(dentry)) == sizeof(dentry);
}

void Fat32Entry::mark_entry_as_nomore(Fat32Entry& e) const {
    e.name = DirectoryEntryFat32::DIR_ENTRY_NO_MORE; // mark entry as last one
    e.update_entry_info_in_parent_dir();
}

void Fat32Entry::mark_next_entry_as_nomore(const Fat32Entry& e) const {
    // if next entry is first entry in a cluster - CLUSTER_END_OF_DIRECTORY will mark the end of directory entries
    u32 num_entries_per_cluster = fat_data.get_sectors_per_cluster() * FAT32ENTRIES_PER_SECTOR;
    u32 next_entry_index = e.parent_index + 1;
    if ((next_entry_index % num_entries_per_cluster) == 0)
        return;

    Fat32Entry no_more(fat_table, fat_data);
    no_more.parent_data = e.parent_data;
    no_more.parent_index = e.parent_index + 1;
    mark_entry_as_nomore(no_more);
}

/**
 * @brief   Is this entry initialized and actually points to some entry in filesystem?
 */
bool Fat32Entry::is_initialized() const {
    return !parent_data.empty();
}

} /* namespace filesystem */
