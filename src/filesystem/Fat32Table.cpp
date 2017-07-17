/**
 *   @file: Fat32Table.cpp
 *
 *   @date: Jul 10, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Table.h"

namespace filesystem {

Fat32Table::Fat32Table(drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Table::setup(u32 fat_start_in_sectors, u16 sector_size, u32 fat_size_in_sectors) {
    FAT_START_IN_SECTORS = fat_start_in_sectors;
    FAT_ENTRIES_PER_SECTOR = sector_size / sizeof(FatTableEntry);
    FAT_SIZE_IN_SECTORS = fat_size_in_sectors;
}

u32 Fat32Table::get_used_space_in_clusters() const {
    FatTableEntry table[FAT_ENTRIES_PER_SECTOR];
    u32 used_clusters = 0;

    for (u32 sector = 0; sector < FAT_SIZE_IN_SECTORS; sector++) {
        read_fat_table_sector(sector, table, sizeof(table));
        for (u32 entry_no = 0; entry_no < FAT_ENTRIES_PER_SECTOR; entry_no++) {
            if (sector == 0 && entry_no < CLUSTER_FIRST_VALID)
                continue; // first two entries in FAT are reserved just as first two data clusters and so are not accounted here

            u32 cluster = table[entry_no] & FAT32_CLUSTER_28BIT_MASK;
            if (cluster != CLUSTER_UNUSED)
                used_clusters++;
        }
    }

    return used_clusters;
}

u32 Fat32Table::get_next_cluster(u32 cluster) const {
    FatTableEntry fat_buffer[FAT_ENTRIES_PER_SECTOR];

    u32 fat_sector_for_current_cluster = cluster / FAT_ENTRIES_PER_SECTOR;
    read_fat_table_sector(fat_sector_for_current_cluster, fat_buffer, sizeof(fat_buffer));
    u32 fat_offset_in_sector_for_current_cluster = cluster % FAT_ENTRIES_PER_SECTOR;

    return fat_buffer[fat_offset_in_sector_for_current_cluster] & FAT32_CLUSTER_28BIT_MASK;
}

/**
 * @brief   Find previous element in single linked list
 * @param   first_cluster   Head
 * @param   cluster Element for which we want to find previous element
 * @return  Previous cluster if exists, CLUSTER_UNUSED otherwise
 */
u32 Fat32Table::get_prev_cluster(u32 first_cluster, u32 cluster) const {
    u32 prev_cluster;
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
 * @brief   Set cluster.next
 */
bool Fat32Table::set_next_cluster(u32 cluster, u32 next_cluster) const {
    FatTableEntry fat_buffer[FAT_ENTRIES_PER_SECTOR];

    u32 fat_sector_for_cluster = cluster / FAT_ENTRIES_PER_SECTOR;
    if (!read_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    u32 fat_offset_in_sector_for_current_cluster = cluster % FAT_ENTRIES_PER_SECTOR;

    fat_buffer[fat_offset_in_sector_for_current_cluster] = next_cluster;
    if (!write_fat_table_sector(fat_sector_for_cluster, fat_buffer, sizeof(fat_buffer)))
        return false;

    return true;
}

bool Fat32Table::is_allocated_cluster(u32 cluster) const {
    return cluster >= CLUSTER_FIRST_VALID && cluster <= CLUSTER_LAST_VALID;
}

bool Fat32Table::read_fat_table_sector(u32 sector, void* data, u32 size) const {
    return hdd.read28(FAT_START_IN_SECTORS + sector, data, size);
}

bool Fat32Table::write_fat_table_sector(u32 sector, void const* data, u32 size) const {
    return hdd.write28(FAT_START_IN_SECTORS + sector, data, size);
}

u32 Fat32Table::alloc_cluster_for_directory() const {
    FatTableEntry table[FAT_ENTRIES_PER_SECTOR];

    for (u32 sector = 0; sector < FAT_SIZE_IN_SECTORS; sector++) {
        read_fat_table_sector(sector, table, sizeof(table));
        for (u32 entry_no = 0; entry_no < FAT_ENTRIES_PER_SECTOR; entry_no++) {
            if (sector == 0 && entry_no < CLUSTER_FIRST_VALID)
                continue; // first two entries in FAT are reserved just as first two data clusters and so are not accounted here

            u32 cluster = table[entry_no] & FAT32_CLUSTER_28BIT_MASK;
            if (cluster == CLUSTER_UNUSED) {    // free cluster found
                // alloc directory end cluster in fat table
                table[entry_no] = CLUSTER_END_OF_CHAIN;
                write_fat_table_sector(sector, table, sizeof(table));

                // return allocated cluster number
                return sector * FAT_ENTRIES_PER_SECTOR + entry_no;
            }
        }
    }

    return CLUSTER_UNUSED; // no free cluster found
}

/**
 * FAT table is a linked list of subsequent clusters in use. Set the pointers to 0 so the clusters are free to be used again
 * @param   cluster First cluster in the list to be freed
 */
void Fat32Table::free_cluster_chain(u32 cluster) const {
    FatTableEntry fat_buffer[FAT_ENTRIES_PER_SECTOR];
    while (is_allocated_cluster(cluster)) {
        u32 fat_sector = cluster / FAT_ENTRIES_PER_SECTOR;
        read_fat_table_sector(fat_sector, fat_buffer, sizeof(fat_buffer));
        u32 fat_offset = cluster % FAT_ENTRIES_PER_SECTOR;

        u32 next_cluster = fat_buffer[fat_offset] & FAT32_CLUSTER_28BIT_MASK;
        fat_buffer[fat_offset] = CLUSTER_UNUSED; // free cluster in fat table

        write_fat_table_sector(fat_sector, fat_buffer, sizeof(fat_buffer));
        cluster = next_cluster;
    }
}

} /* namespace filesystem */
