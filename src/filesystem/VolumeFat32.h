/**
 *   @file: VolumeFat32.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 *    @see: https://www.pjrc.com/tech/8051/ide/fat32.html
 */




/*
FAT32 partition is essentially made of 3 structures:
-FAT header, which says how big is the partition, where the fat table starts, where the file data starts.
-FAT table, which is a big array of 32-bit numbers. These numbers are cluster numbers.
-FAT data, which is a huge array of 4KB clusters (cluster can be other size, we read this info from the header).

In Fat32, any data is stored in form of linked clusters. Linked just like in LinkedList of char[4096].
In clusters we store data and 1 cluster in the smallest unit of allocation.
Need store 4KB data? Use single cluster. Need 8KB? use 2 linked clusters. Need 12KB? use 3 linked clusters. And so on.

Now how to read the root directory in Fat32?
Well, root position (actually root cluster number, as all data in Fat32 is allocated in form of clusters)
can be read from the FAT header. I will most likely be cluster number 2.
Clusters 0 and 1 are reserved, so cluster 2 is the first usable one, perfect match for the root.
The formula for byte address is:
    data_start_in_bytes + (cluster_number -2) * cluster_size_in_bytes. -2 since 2 reserved clusters.

But of course the disk is allocated in form of sectors(512 bytes), so more useful formula is:
    data_start_in_sectors + (cluster_number -2) * cluster_size_in_sectors. -2 since 2 reserved clusters.

And what can we find at cluster 2? An array of 32 byte long DirectoryEntry structures. Each one holding entry name,
file/directory flag, and number of cluster that holds its data (for file - file contents, for dir - array of entries).
Now as we have 4096 byte cluster and 32 bytes DirectoryEntry, its easy to tell that single dir can only hold 128 entries.
But if we need to create more entries, we can always link another cluster...and another...and another...

How the cluster linking works?
Cluster itself doesnt hold the "next_cluster_no" information. This information is stored in Fat table. How it works?
Imagine you have your root dir at cluster 2 and you want to know what is its next linked cluster. What you do is:
next_cluster_number = fat_table[2] & 0x0FFFFFFF. Mind we use only 28 bits, as Fat32 use actually only 28 bit
That's it! Fat table addressed with cluster number returns the next linked cluster number.
0x00000000 - cluster is unused
0x00000002 - 0x0FFFFFF7 - data cluster
0x0FFFFFF8 - 0x0FFFFFFF - last cluster in chain


CREATE ENTRY (FILE OR DIR)
    read parent_dir entry for provided path, if not exists -> return
    check if entry with such name already exists, if true -> return
    prepare new_entry
    find slot for writing our entry in parent_dir data:
        if parent dir has no data cluster allocated:        // parent_dir.data == 0
            allocate new_cluster                            // fat_table.allocate
            zero the new_cluster                            // fat_data.zero
            set the new_cluster as parent_dir data          // parent_dir.data = new_cluster, fat_data.write(parent_dir)
            write our new_entry at pos 0 of new_cluster     // new_entry.cluster = new_cluster, new_entry.index = 0, fat_data.write(new_entry)
            write NO_MORE entry at pos 1 of new_cluster     // no_more.cluster = new_cluster, no_more.index = 1, fat_data.write(no_more)

        if parent_dir has UNUSED entry:
            write our new_entry at position of UNUSED       // new_entry.cluster = unused.cluster, new_entry.index = unused.index, fat_data.write(new_entry)

        if parent_dir has NO_MORE entry:
            write our new_entry at position of NO MORE      // new_entry.cluster = no_more.cluster, new_entry.index = no_more.index, fat_data.write(new_entry)
            if there is next entry - write NO MORE          // no_more.index++, fat_data.write(no_more)
                                                            // if no_more.index >= max_entries_per_cluster -> fail silently
        otherwise (all entries are used):
            allocate new_cluster                            // see above
            zero the new_cluster
            set the new_cluster as parent_dir last cluster
            write our new_entry at pos 0 of new_cluster
            write NO_MORE entry at pos 1 of new_cluster


DELETE ENTRY (FILE OR DIR)
    read parent_dir entry for provided path, if not exists -> return
    read entry itself for provided path, if not exists -> return

    if entry is DIR and entry is not empty:
        return

    if entry is FILE:
        free entry.data                                 // fat_table.free_cluster_chain(entry.data)

    if entry is the last one in parent_dir:             // no_more_entires_after(parent_dir, entry)
        mark entry as NO_MORE                           // fat_data.mark_as_no_more(parent_dir, entry)
    else
        mark entry as UNUSED                            // fat_data.mark_as_unused(parent_dir, entry)

    if cluster where entry was is now empty:            // fat_data.is_directory_cluster_empty(entry.cluster)
        detach cluster from parent_dir                  // detach_directory_cluster(parent_dir, entry.cluster)


MOVE ENTRY (FILE OR DIR)
    check if source entry exists
        if not -> return false

    check if destination path exists
        if not -> return false

    check if destination entry exists
        if yes -> return false

    get source entry (src)
    create destination entry based on source entry including data cluster (dst)
    src.data_cluster = UNUSED
    delete entry(src)


READ FILE ENTRY
    locate reading start point for "position" byte:
        get cluster
        calculate sector_in_cluster
        calculate byte_in_sector

    while (remaining_bytes_to_read > 0) AND (cluster is valid data cluster)
        for each sector >= sector_in_cluster
            read data sector bytes into output buffer
            remaining_bytes_to_read -= bytes_read
            total_bytes_read += bytes_read

        cluster = next_cluster(cluster)
        sector_in_cluster = 0

    position += total_bytes_read


WRITE FILE ENTRY
    locate writing start point for "position" byte:
        get or allocate cluster
        calculate sector_in_cluster
        calculate byte_in_sector

    while (remaining_bytes_to_write > 0) AND (cluster is valid data cluster)
        for each sector >= sector_in_cluster
            write bytes into sector
            remaining_bytes_to_write -= bytes_written
            total_bytes_written += bytes_written

        cluster = attach_next_cluster(cluster)
        sector_in_cluster = 0

    position += total_bytes_written
    size = max(size, position)
    write file entry to hdd as the size might have been increased

*/


#ifndef SRC_FILESYSTEM_VOLUMEFAT32_H_
#define SRC_FILESYSTEM_VOLUMEFAT32_H_

#include "KernelLog.h"
#include "Fat32Table.h"
#include "Fat32Data.h"
#include "kstd.h"
#include "AtaDriver.h"

namespace filesystem {


/**
 * @name    VolumeFat32
 * @brief   Fat32 volume/partition abstraction.
 */
class VolumeFat32 {
public:
    VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors);
    kstd::string get_label() const;
    kstd::string get_type() const;
    u32 get_size_in_bytes() const;
    u32 get_used_space_in_bytes() const;
    u32 get_used_space_in_clusters() const;

    bool get_entry(const kstd::string& unix_path, SimpleDentryFat32& e) const;
    u32 read_file_entry(SimpleDentryFat32& file, void* data, u32 count) const;
    u32 write_file_entry(SimpleDentryFat32& file, void const * data, u32 count) const;
    void seek_file_entry(SimpleDentryFat32& file, u32 position) const;
    void trunc_file_entry(SimpleDentryFat32& file, u32 new_size) const;
    EnumerateResult enumerate_directory_entry(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry_found) const;
    bool create_entry(const kstd::string& unix_path, bool directory) const;
    bool create_entry(const kstd::string& unix_path, bool directory, SimpleDentryFat32& out) const;
    bool delete_entry(const kstd::string& unix_path) const;
    bool move_entry(const kstd::string& unix_path_from, const kstd::string& unix_path_to) const;

private:
    SimpleDentryFat32 get_root_dentry() const;
    bool get_entry_for_name(const SimpleDentryFat32& parent_dir, const kstd::string& filename, SimpleDentryFat32& out) const;
    bool alloc_first_dir_cluster_and_alloc_entry(SimpleDentryFat32& parent_dir, SimpleDentryFat32& e) const;
    bool alloc_last_dir_cluster_and_alloc_entry(SimpleDentryFat32& parent_dir, SimpleDentryFat32& e) const;
    bool try_alloc_entry_in_free_dir_slot(const SimpleDentryFat32& parent_dir, SimpleDentryFat32 &e) const;
    u32 attach_new_directory_cluster(SimpleDentryFat32& parent_dir) const;
    void detach_directory_cluster(const SimpleDentryFat32& dentry, u32 cluster) const;
    bool alloc_entry_in_directory(SimpleDentryFat32& parent_dir, SimpleDentryFat32 &e) const;
    bool is_file_empty(const SimpleDentryFat32& e) const;
    bool is_directory_empty(const SimpleDentryFat32& e) const;
    bool is_no_more_entires_after(const SimpleDentryFat32& parent_dir, const SimpleDentryFat32& entry) const;
    u32 alloc_first_file_cluster(SimpleDentryFat32& file) const;
    u32 alloc_next_file_cluster(u32 cluster) const;

    u32 read_file_data(SimpleDentryFat32& file, void* data, u32 count) const;
    u32 read_cluster_data(u16 byte_in_sector, u8 sector_in_cluster, u32 cluster, u8* data, u32 count) const;
    u32 write_file_data(SimpleDentryFat32& file, const void* data, u32 count) const;
    u32 get_cluster_for_write(SimpleDentryFat32& file) const;
    u32 write_cluster_data(u16 byte_in_sector, u8 sector_in_cluster, u32 cluster, const u8* data, u32 count) const;
    bool is_next_cluster_needed(u32 next_read_position) const;

    drivers::AtaDevice& hdd;
    VolumeBootRecordFat32 vbr;
    Fat32Table fat_table;
    Fat32Data fat_data;
    utils::KernelLog& klog;

    bool bootable;
    u32 partition_offset_in_sectors;
    u32 partition_size_in_sectors;

    u32 fat_start;
    u32 data_start;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VOLUMEFAT32_H_ */
