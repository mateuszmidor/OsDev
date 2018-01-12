/**
 *   @file: Fat32Table.h
 *
 *   @date: Jul 10, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_FAT32TABLE_H_
#define SRC_FILESYSTEM_FAT32_FAT32TABLE_H_

#include "AtaDriver.h"
#include "KernelLog.h"

namespace filesystem {

class Fat32Table {
public:
    Fat32Table(const drivers::AtaDevice& hdd);
    void setup(u32 fat_start_in_sectors, u16 sector_size, u8 sectors_per_cluster, u32 fat_size_in_sectors);

    u32 get_used_space_in_clusters() const;
    u32 get_next_cluster(u32 cluster) const;
    u32 get_prev_cluster(u32 first_cluster, u32 cluster) const;
    u32 get_last_cluster(u32 cluster) const;
    u32 find_cluster_for_byte(u32 first_cluster, u32 byte_number) const;
    u32 resize_cluster_chain(u32 first_cluster, u32 num_bytes) const;
    bool set_next_cluster(u32 cluster, u32 next_cluster) const;
    bool is_allocated_cluster(u32 cluster) const;
    u32 alloc_cluster() const;
    void free_cluster_chain(u32 e_cluster) const;
    u32 detach_cluster(u32 first_cluster, u32 cluster) const;

    static const u32 CLUSTER_UNUSED             = 0;            // In Fat32 table, unused clusters are marked as 0
    static const u32 CLUSTER_FIRST_VALID        = 2;            // Clusters 0 and 1 are reserved, 2 usually is the cluster of root dir
    static const u32 CLUSTER_LAST_VALID         = 0x0FFFFFF7;
    static const u32 CLUSTER_END_OF_CHAIN       = 0x0FFFFFF8;   // Such entry in Fat32 table indicates we've reached last cluster in the chain
//    static const u32 CLUSTER_END_OF_DIRECTORY   = 0x0FFFFFFF;   // Such entry in Fat32 table indicates we've reached the last cluster in dir chain
    static const u32 FAT32_CLUSTER_28BIT_MASK   = 0x0FFFFFFF;   // Fat32 table cluster index actually use 28 bits, highest 4 bits should be ignored

private:
    using FatTableEntry = u32;  // FatEntry represents cluster index which is 32 bit (28 actually used)
    bool read_fat_table_sector(u32 sector, void* data, u32 size) const;
    bool write_fat_table_sector(u32 sector, void const* data, u32 size) const;

    const drivers::AtaDevice    hdd;
    logging::KernelLog&         klog;
    u16                         fat_entries_per_sector  = 0;
    u32                         fat_start_in_sectors    = 0;
    u32                         fat_size_in_sectors     = 0;
    u16                         bytes_per_sector        = 0;
    u8                          sectors_per_cluster     = 0;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32_FAT32TABLE_H_ */
