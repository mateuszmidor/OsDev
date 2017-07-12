/**
 *   @file: Fat32Table.h
 *
 *   @date: Jul 10, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32TABLE_H_
#define SRC_FILESYSTEM_FAT32TABLE_H_

#include "AtaDriver.h"

namespace filesystem {

class Fat32Table {
public:
    Fat32Table(drivers::AtaDevice& hdd);
    void setup(u16 sector_size, u32 fat_start_in_sectors, u32 fat_size_in_sectors);

    u32 get_used_space_in_clusters() const;
    u32 get_next_cluster(u32 cluster) const;
    u32 get_prev_cluster(u32 first_cluster, u32 cluster) const;
    bool set_next_cluster(u32 cluster, u32 next_cluster) const;
    bool is_allocated_cluster(u32 cluster) const;
    u32 alloc_cluster_for_directory() const;
    void free_cluster_chain(u32 e_cluster) const;

    static const u32 CLUSTER_UNUSED             = 0;            // In Fat32 table, unused clusters are marked as 0
    static const u32 CLUSTER_FIRST_VALID        = 2;            // Clusters 0 and 1 are reserved, 2 usually is the cluster of root dir
    static const u32 CLUSTER_END_OF_FILE        = 0x0FFFFFF8;   // Such entry in Fat32 table indicates we've reached last cluster in file chain
    static const u32 CLUSTER_END_OF_DIRECTORY   = 0x0FFFFFFF;   // Such entry in Fat32 table indicates we've reached the last cluster in dir chain
    static const u32 FAT32_CLUSTER_28BIT_MASK   = 0x0FFFFFFF;   // Fat32 table cluster index actually use 28 bits, highest 4 bits should be ignored

private:
    using FatTableEntry = u32;  // FatEntry represents cluster index which is 32 bit (28 actually used)

    drivers::AtaDevice& hdd;
    u16 FAT_ENTRIES_PER_SECTOR  = 0;
    u32 FAT_START_IN_SECTORS    = 0;
    u32 FAT_SIZE_IN_SECTORS     = 0;


    bool read_fat_table_sector(u32 sector, void* data, u32 size) const;
    bool write_fat_table_sector(u32 sector, void const* data, u32 size) const;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32TABLE_H_ */
