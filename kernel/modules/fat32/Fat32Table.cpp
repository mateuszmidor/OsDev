/**
 *   @file: Fat32Table.cpp
 *
 *   @date: Jul 10, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Table.h"
#include "Requests.h"

namespace filesystem {
namespace fat32 {

Fat32Table::Fat32Table(const drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Table::setup(u32 fat_start_in_sectors, u16 sector_size, u8 sectors_per_cluster, u32 fat_size_in_sectors) {
    this->fat_start_in_sectors = fat_start_in_sectors;
    this->fat_entries_per_sector = sector_size / sizeof(FatTableEntry);
    this->fat_size_in_sectors = fat_size_in_sectors;
    this->bytes_per_sector = sector_size;
    this->sectors_per_cluster = sectors_per_cluster;
}

u32 Fat32Table::get_used_space_in_clusters() const {
    FatTableEntry table[fat_entries_per_sector];
    u32 used_clusters = 0;

    for (u32 sector = 0; sector < fat_size_in_sectors; sector++) {
        read_fat_table_sector(sector, table, sizeof(table));
        for (u32 entry_no = 0; entry_no < fat_entries_per_sector; entry_no++) {
            if (sector == 0 && entry_no < CLUSTER_FIRST_VALID)
                continue; // first two entries in FAT are reserved just as first two data clusters and so are not accounted here

            u32 cluster = table[entry_no] & FAT32_CLUSTER_28BIT_MASK;
            if (cluster != CLUSTER_UNUSED)
                used_clusters++;
        }
    }

    return used_clusters;
}

/**
 * @brief   Get next cluster in the chain or Fat32Table::CLUSTER_END_OF_CHAIN if end of chain reached
 */
u32 Fat32Table::get_next_cluster(u32 cluster) const {
    FatTableEntry fat_buffer[fat_entries_per_sector];

    u32 fat_sector_for_current_cluster = cluster / fat_entries_per_sector;
    read_fat_table_sector(fat_sector_for_current_cluster, fat_buffer, sizeof(fat_buffer));
    u32 fat_offset_in_sector_for_current_cluster = cluster % fat_entries_per_sector;

    return fat_buffer[fat_offset_in_sector_for_current_cluster] & FAT32_CLUSTER_28BIT_MASK;
}

/**
 * @brief   Find previous element in single linked list
 * @param   first_cluster   Head
 * @param   cluster Element for which we want to find previous element
 * @return  Previous cluster if exists, CLUSTER_UNUSED otherwise
 */
u32 Fat32Table::get_prev_cluster(u32 first_cluster, u32 cluster) const {
    u32 prev_cluster = CLUSTER_END_OF_CHAIN;
    u32 curr_cluster = first_cluster;
    while (curr_cluster != cluster)  {
        if (!is_allocated_cluster(curr_cluster))
            return CLUSTER_UNUSED;

        prev_cluster = curr_cluster;
        curr_cluster = get_next_cluster(curr_cluster);
    }
    return prev_cluster;
}

/**
 * @brief   Get last cluster no in chain
 */
u32 Fat32Table::get_last_cluster(u32 cluster) const {
    u32 prev_cluster = CLUSTER_END_OF_CHAIN;
    u32 curr_cluster = cluster;
    while (is_allocated_cluster(curr_cluster))  {
        prev_cluster = curr_cluster;
        curr_cluster = get_next_cluster(curr_cluster);
    }
    return prev_cluster;
}

/**
 * @brief   Get cluster where the "byte_number" byte resides
 */
u32 Fat32Table::find_cluster_for_byte(u32 first_cluster, u32 byte_number) const {
    // calc cluster number in the chain that the "byte_number" byte is located in
    u32 cluster_no = byte_number / (bytes_per_sector * sectors_per_cluster);
    u32 cluster = first_cluster;

    // follow cluster chain until "last_cluster_no" is reached
    while (cluster_no > 0 && is_allocated_cluster(cluster)) {
        cluster = get_next_cluster(cluster);
        cluster_no--;
    }

    return cluster;
}

/**
 * @brief   Resize cluster chain to accomodate "num_bytes" of data
 * @return  first_cluster if "num_bytes" > 0, CLUSTER_UNUSED otherwise
 */
u32 Fat32Table::resize_cluster_chain(u32 first_cluster, u32 num_bytes) const {
    // chain should be freed
    if (num_bytes == 0) {
        free_cluster_chain(first_cluster);
        return CLUSTER_UNUSED;
    }

    // chain is empty but should grow
    if (first_cluster == CLUSTER_UNUSED)
        first_cluster = alloc_cluster();

    // calc last cluster number
    u32 last_cluster_no = (num_bytes - 1) / (bytes_per_sector * sectors_per_cluster); // 5 = 0, 4096 = 0, 4097 = 1
    u32 prev_cluster = CLUSTER_END_OF_CHAIN;
    u32 target_cluster = first_cluster;

    // follow/make cluster chain until "last_cluster_no" is reached
    while (last_cluster_no > 0) {
        prev_cluster = target_cluster;
        target_cluster = get_next_cluster(target_cluster);

        if (!is_allocated_cluster(target_cluster)) {
            target_cluster = alloc_cluster(); // should also clear the data
            set_next_cluster(prev_cluster, target_cluster);
        }

        last_cluster_no--;
    }

    // check if there is more clusters after target_cluster and if so -free them
    u32 cluster_after_end = get_next_cluster(target_cluster);
    if (is_allocated_cluster(cluster_after_end)) {
        free_cluster_chain(cluster_after_end);
        set_next_cluster(target_cluster, CLUSTER_END_OF_CHAIN);
    }

    return first_cluster;
}

/**
 * @brief   Set cluster.next
 */
bool Fat32Table::set_next_cluster(u32 cluster, u32 next_cluster) const {
    FatTableEntry fat_buffer[fat_entries_per_sector];

    u32 fat_sector_for_cluster = cluster / fat_entries_per_sector;
    if (!read_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    u32 fat_offset_in_sector_for_current_cluster = cluster % fat_entries_per_sector;

    fat_buffer[fat_offset_in_sector_for_current_cluster] = next_cluster;
    if (!write_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    return true;
}

bool Fat32Table::is_allocated_cluster(u32 cluster) const {
    return cluster >= CLUSTER_FIRST_VALID && cluster <= CLUSTER_LAST_VALID;
}

bool Fat32Table::read_fat_table_sector(u32 sector, void* data, u32 size) const {
    return hdd.read28(fat_start_in_sectors + sector, data, size);
}

bool Fat32Table::write_fat_table_sector(u32 sector, void const* data, u32 size) const {
    return hdd.write28(fat_start_in_sectors + sector, data, size);
}

/**
 * @brief   Allocate single cluster and set it in Fat table as CLUSTER_END_OF_CHAIN
 * @return  Newly allocated cluster if success, Fat32Table::CLUSTER_END_OF_CHAIN otherwise
 */
u32 Fat32Table::alloc_cluster() const {
    FatTableEntry table[fat_entries_per_sector];

    for (u32 sector = 0; sector < fat_size_in_sectors; sector++) {
        read_fat_table_sector(sector, table, sizeof(table));
        for (u32 entry_no = 0; entry_no < fat_entries_per_sector; entry_no++) {
            if (sector == 0 && entry_no < CLUSTER_FIRST_VALID)
                continue; // first two entries in FAT are reserved just as first two data clusters and so are not accounted here

            u32 cluster = table[entry_no] & FAT32_CLUSTER_28BIT_MASK;
            if (cluster == CLUSTER_UNUSED) {    // free cluster found
                // alloc directory end cluster in fat table
                table[entry_no] = CLUSTER_END_OF_CHAIN;
                write_fat_table_sector(sector, table, sizeof(table));

                // return allocated cluster number
                return sector * fat_entries_per_sector + entry_no;
            }
        }
    }

    requests->log("Fat32Table::alloc_cluster: no free cluster to allocate found\n");
    return CLUSTER_END_OF_CHAIN; // no free cluster found
}

/**
 * FAT table is a linked list of subsequent clusters in use. Set the pointers to 0 so the clusters are free to be used again
 * @param   cluster First cluster in the list to be freed
 */
void Fat32Table::free_cluster_chain(u32 cluster) const {
    FatTableEntry fat_buffer[fat_entries_per_sector];
    while (is_allocated_cluster(cluster)) {
        u32 fat_sector = cluster / fat_entries_per_sector;
        read_fat_table_sector(fat_sector, fat_buffer, sizeof(fat_buffer));
        u32 fat_offset = cluster % fat_entries_per_sector;

        u32 next_cluster = fat_buffer[fat_offset] & FAT32_CLUSTER_28BIT_MASK;
        fat_buffer[fat_offset] = CLUSTER_UNUSED; // free cluster in fat table

        write_fat_table_sector(fat_sector, fat_buffer, sizeof(fat_buffer));
        cluster = next_cluster;
    }
}

/**
 * @brief   Detach cluster from cluster chain; this works the same as deleting element from a single linked list
 * @param   first_cluster List head
 * @param   cluster List element to be deleted
 * @return  New list head or CLUSTER_UNUSED if cluster was the only list element
 */
u32 Fat32Table::detach_cluster(u32 first_cluster, u32 cluster) const {
    // first, remember next cluster in the chain
    u32 next_cluster = get_next_cluster(cluster);

    // removing first cluster? update head
    if (cluster == first_cluster) {
        first_cluster = is_allocated_cluster(next_cluster) ? next_cluster : Fat32Table::CLUSTER_UNUSED;
    }
    // removing not first cluster? update previous cluster to point to the next cluster effectively removing current link
    else {
        // find one cluster before "cluster" and link it with next_cluster, detaching cluster
        u32 prev_cluster = get_prev_cluster(first_cluster, cluster);
        set_next_cluster(prev_cluster, next_cluster);
    }

    set_next_cluster(cluster, Fat32Table::CLUSTER_UNUSED);    // this cluster is free now
    return first_cluster;
}

} /* namespace fat32 */
} /* namespace filesystem */
