/**
 *   @file: Fat32ClusterChain.h
 *
 *   @date: Aug 3, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_FAT32CLUSTERCHAIN_H_
#define SRC_FILESYSTEM_FAT32_FAT32CLUSTERCHAIN_H_

#include "Fat32Data.h"
#include "Fat32Table.h"
#include "KernelLog.h"

namespace filesystem {

/**
 * @brief   This class is an abstraction for Fat32 data storage that is organized in form of singly linked list of so called clusters that are eg. 4096 bytes long
 */
class Fat32ClusterChain {
public:
    Fat32ClusterChain(const Fat32Table& fat_table, const Fat32Data& fat_data, u32 head_cluster = Fat32Table::CLUSTER_UNUSED, u32 size = 0);
    Fat32ClusterChain& operator=(const Fat32ClusterChain& other);
    void free();
    void resize(u32 new_size_in_bytes);
    bool is_empty() const;
    u32 get_head() const;
    u32 get_tail();
    u32 get_size() const;
    u32 get_position() const;
    bool seek(u32 new_position);
    bool attach_cluster();
    bool attach_cluster_and_zero_it();
    bool detach_cluster(u32 cluster);
    u32 read(void* data, u32 count);
    u32 write(const void* data, u32 count);

private:
    u32 get_cluster_for_write();

    const Fat32Table    fat_table;
    const Fat32Data     fat_data;
    logging::KernelLog& klog;
    u32                 head_cluster;
    u32                 tail_cluster;
    u32                 current_cluster;
    u32                 current_byte;
    u32                 size;

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32_FAT32CLUSTERCHAIN_H_ */
