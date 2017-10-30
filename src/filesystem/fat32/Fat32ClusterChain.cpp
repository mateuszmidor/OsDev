/**
 *   @file: Fat32ClusterChain.cpp
 *
 *   @date: Aug 3, 2017
 * @author: Mateusz Midor
 */

#include "Fat32ClusterChain.h"

using namespace kstd;
namespace filesystem {

Fat32ClusterChain::Fat32ClusterChain(const Fat32Table& fat_table, const Fat32Data& fat_data, u32 start_cluster, u32 size) :
    fat_table(fat_table),
    fat_data(fat_data),
    head_cluster(start_cluster),
    tail_cluster(Fat32Table::CLUSTER_UNUSED),
    current_cluster(start_cluster),
    current_byte(0),
    size(size),
    klog(logging::KernelLog::instance())
{
}

Fat32ClusterChain& Fat32ClusterChain::operator=(const Fat32ClusterChain& other) {
    new (this) Fat32ClusterChain(other);
    return *this;
}

/**
 * @brief   Release all cluster chain elements, effectively making them free to use
 */
void Fat32ClusterChain::free() {
    resize(0);
    current_byte = 0;
    current_cluster = Fat32Table::CLUSTER_UNUSED;
}

/**
 * @brief   Resize cluster chain to accommodate "num_bytes".
 *          If extending, new bytes are not zeroed.
 *          If shrinking, spare clusters are fried.
 *          In any case, current position is not changed.
 */
void Fat32ClusterChain::resize(u32 new_size_in_bytes) {
    head_cluster = fat_table.resize_cluster_chain(head_cluster, new_size_in_bytes);
    tail_cluster = Fat32Table::CLUSTER_UNUSED;
    size = new_size_in_bytes;
}

/**
 * @brief   Is this cluster chain empty, meaning it contains no clusters?
 */
bool Fat32ClusterChain::is_empty() const {
    return head_cluster == Fat32Table::CLUSTER_UNUSED;
}

/**
 * @brief   Get first cluster in the chain
 */
u32 Fat32ClusterChain::get_head() const {
    return head_cluster;
}

/**
 * @brief   Get or find the last cluster in the chain
 */
u32 Fat32ClusterChain::get_tail() {
    if (head_cluster == Fat32Table::CLUSTER_UNUSED)
        return Fat32Table::CLUSTER_UNUSED;

    if (tail_cluster == Fat32Table::CLUSTER_UNUSED)
        tail_cluster = fat_table.get_last_cluster(head_cluster);

    return tail_cluster;
}

u32 Fat32ClusterChain::get_size() const {
    return size;
}

u32 Fat32ClusterChain::get_position() const {
    return current_byte;
}

bool Fat32ClusterChain::seek(u32 new_position) {
    if (new_position > size)
        return false;

    if (new_position == current_byte)
        return true;

    current_cluster = fat_table.find_cluster_for_byte(head_cluster, new_position);
    current_byte = new_position;

    return true;
}

/**
 * @brief   Alloc new cluster and attach it at the chain end
 * @return  True if successfully allocated and attached, False otherwise
 */
bool Fat32ClusterChain::attach_cluster() {
    u32 new_cluster = fat_table.alloc_cluster();

    // check for allocation failure
    if (new_cluster == Fat32Table::CLUSTER_END_OF_CHAIN)
        return false;

    // if chain has no head yet, attach new_cluster as head and update tail and current cluster
    if (head_cluster == Fat32Table::CLUSTER_UNUSED) {
        head_cluster = new_cluster;
        tail_cluster = new_cluster;
        current_cluster = new_cluster;
    } else
    // if head present, attach to tail
    {
        u32 old_tail = get_tail();
        fat_table.set_next_cluster(old_tail, new_cluster);
        tail_cluster = new_cluster;
    }

    return true;
}

/**
 * @brief   Alloc new cluster and attach it at the chain end, then zero all its bytes
 * @return  True if successfully allocated and attached, False otherwise
 */
bool Fat32ClusterChain::attach_cluster_and_zero_it() {
    if (!attach_cluster())
        return false;

    fat_data.clear_data_cluster(get_tail());
    return true;
}

/**
 * @brief   Detach given cluster and reset current position
 */
bool Fat32ClusterChain::detach_cluster(u32 cluster) {
    head_cluster = fat_table.detach_cluster(head_cluster, cluster);
    tail_cluster = Fat32Table::CLUSTER_UNUSED;
    current_cluster = Fat32Table::CLUSTER_UNUSED;
    current_byte = 0;
    return true;
}

/**
 * @brief   Read a maximum of "count" bytes, starting from file.position, into data buffer
 * @param   data  Buffer that has at least "count" capacity
 * @param   count Number of bytes to read
 * @return  Number of bytes actually read
 */
u32 Fat32ClusterChain::read(void* data, u32 count) {
    if (is_empty()) {
        klog.format("Fat32ClusterChain::read: empty cluster chain\n");
        return 0;
    }

    if (current_byte > size) {
        klog.format("Fat32ClusterChain::read: tried reading after end of cluster chain\n");
        return 0;
    }

    // 1. setup reading status constants and variables
    const u32 MAX_BYTES_TO_READ = size - current_byte;
    u32 total_bytes_read = 0;
    u32 remaining_bytes_to_read = min(count, MAX_BYTES_TO_READ);

    // 2. locate reading start point
    u32 position_in_cluster = current_byte; // modulo CLUSTER_SIZE_IN_BYTES but this is done in fat_data.read_data_cluster anyway
    u32 cluster = current_cluster;

    // 3. follow cluster chain and read data from sectors until requested number of bytes is read
    u8* dst = (u8*) data;
    while (fat_table.is_allocated_cluster(cluster)) {
        // read the cluster until end of cluster or requested number of bytes is read
        u32 count = fat_data.read_data_cluster(position_in_cluster, cluster, dst, remaining_bytes_to_read);
        remaining_bytes_to_read -= count;
        total_bytes_read += count;
        dst += count;

        // move on to the next cluster if needed
        if (fat_data.is_cluster_beginning(current_byte + total_bytes_read)) {
            position_in_cluster = 0;
            cluster = fat_table.get_next_cluster(cluster);
        }

        // stop reading if requested number of bytes is read
        if (remaining_bytes_to_read == 0)
            break;
    }

    // 4. done; update file position
    current_byte += total_bytes_read;
    current_cluster = cluster;
    return total_bytes_read;
}

/**
 * @brief   Write "count" bytes into the file, starting from file.position, enlarging the file size if needed
 * @param   data Data to be written
 * @param   count Number of bytes to be written
 * @return  Number of bytes actually written
 */
u32 Fat32ClusterChain::write(const void* data, u32 count) {
    if ((u64)current_byte + count > 0xFFFFFFFF){
        klog.format("Fat32ClusterChain::write: would exceed Fat32 4GB limit\n");
        return 0;
    }

    // 1. setup writing status variables
    u32 total_bytes_written = 0;
    u32 remaining_bytes_to_write = count;

    // 2. locate writing start point
    u32 position_in_cluster = current_byte;
    u32 cluster = get_cluster_for_write();

    // 3. follow/make cluster chain and write data to sectors until requested number of bytes is written
    const u8* src = (const u8*)data;
    while (fat_table.is_allocated_cluster(cluster)) {
        // write the cluster until end of cluster or requested number of bytes is written
        u32 count = fat_data.write_data_cluster(position_in_cluster, cluster, src, remaining_bytes_to_write);
        remaining_bytes_to_write -= count;
        total_bytes_written += count;

        // stop writing if requested number of bytes is written
        if (remaining_bytes_to_write == 0)
            break;

        // move on to the next cluster
        src += count;
        position_in_cluster = 0;
        cluster = fat_table.get_next_cluster(cluster);
        if (cluster == Fat32Table::CLUSTER_END_OF_CHAIN) {
            attach_cluster();
            cluster = get_tail();
        }
    }

    // 4. done; update file position and size if needed
    current_byte += total_bytes_written;
    current_cluster = (fat_data.is_cluster_beginning(current_byte)) ? fat_table.get_next_cluster(cluster) : cluster;

    if (size < current_byte) {
        size = current_byte;
    }

    return total_bytes_written;
}

/**
 * @brief   Get proper cluster for data writing depending on file status and file position
 * @return  Cluster for writing data
 */
u32 Fat32ClusterChain::get_cluster_for_write() {
    u32 cluster;

    if (fat_table.is_allocated_cluster(current_cluster)) {
        cluster = current_cluster;
        klog.format("Fat32ClusterChain::get_cluster_for_write: reusing cluster %\n", cluster);
    } else {
        attach_cluster();
        cluster = get_tail();
        klog.format("Fat32ClusterChain::get_cluster_for_write: attached new cluster %\n", cluster);
    }

    return cluster;
}
} /* namespace filesystem */
