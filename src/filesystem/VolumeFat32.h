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
    prepare new_entry
    find slot for writing our entry in parent_dir data:
        if parent dir has no data cluster allocated:        // parent_dir.data == 0
            allocate new_cluster                            // fat_table.allocate
            zero the new_cluster                            // fat_data.zero
            set the new_cluster as parent_dir data          // parent_dir.data = new_cluster, fat_data.write(parent_dir)
            write our new_entry at pos 0 of new_cluster     // fat_data.write(cluster, 0, new_entry)
            write NO_MORE entry at pos 1 of new_cluster     // fat_data.write(cluster, 1, NO_MORE_ENTRY)

        if parent_dir has UNUSED entry:
            write our new_entry at position of UNUSED       // fat_data.write(unused.cluster, unused.index, new_entry)

        if parent_dir has NO_MORE entry
            write our new_entry at position of NO MORE      // fat_data.write(nomore.cluster, nomore.index, new_entry)
            if there is next entry - write NO MORE          // fat_data.write(nomore.cluster, nomore.index=1, NO_MORE_ENTRY)
                                                            // if index >= max_entries_per_cluster -> fail silently
        otherwise (all entries are used):
            allocate new_cluster
            zero the new_cluster
            set the new_cluster as parent_dir last cluster
            write our new_entry at pos 0 of new_cluster
            write NO_MORE entry at pos 1 of new_cluster
*/


#ifndef SRC_FILESYSTEM_VOLUMEFAT32_H_
#define SRC_FILESYSTEM_VOLUMEFAT32_H_

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
    u32 read_file_entry(const SimpleDentryFat32& file, void* data, u32 count) const;
    EnumerateResult enumerate_directory_entry(const SimpleDentryFat32& dentry, const OnEntryFound& on_entry_found) const;
    bool create_entry(const kstd::string& unix_path, bool directory) const;
    bool delete_entry(const kstd::string& unix_path) const;

private:
    SimpleDentryFat32 get_root_dentry() const;
    bool get_entry_for_name(const SimpleDentryFat32& dentry, const kstd::string& filename, SimpleDentryFat32& out) const;

    // delete_file stuff
    void remove_dir_cluster_if_empty(const SimpleDentryFat32& dentry, u32 cluster) const;
    bool alloc_entry_in_directory(const SimpleDentryFat32& dir, SimpleDentryFat32 &e) const;
    bool is_directory_empty(const SimpleDentryFat32& e) const;


    drivers::AtaDevice& hdd;
    VolumeBootRecordFat32 vbr;
    Fat32Table fat_table;
    Fat32Data fat_data;

    bool bootable;
    u32 partition_offset_in_sectors;
    u32 partition_size_in_sectors;

    u32 fat_start;
    u32 data_start;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VOLUMEFAT32_H_ */
