/**
 *   @file: Fat32Data.cpp
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "Fat32Data.h"

namespace filesystem {
namespace fat32 {

Fat32Data::Fat32Data(const drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Data::setup(u32 data_start_in_sectors, u16 bytes_per_sector, u8 sectors_per_cluster) {
    this->data_start_in_sectors = data_start_in_sectors;
    this->bytes_per_sector = bytes_per_sector;
    this->sectors_per_cluster = sectors_per_cluster;
}

bool Fat32Data::read_data_sector(u32 cluster, u8 sector_in_cluster, void* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.read28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_in_cluster, data, size);
}

/**
 * @brief Read "size" bytes of data at [cluster][sector_in_cluster][byte_in_sector]
 */
bool Fat32Data::read_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void* data, u32 size) const {
    // read across sectors is not supported
    if (byte_in_sector + size > bytes_per_sector)
        return false;

    // read starts at sector beginning, optimal read
    if (byte_in_sector == 0)
        return read_data_sector(cluster, sector_in_cluster, data, size);


    // read starts somewhere in the middle of sector, need read entire sector and copy from buffer to data
    u8 buff[bytes_per_sector];
    if (!read_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector))
        return false;

    memcpy(data, buff + byte_in_sector, size);
    return true;
}

/**
 * @brief   Read the cluster starting from "position" byte until "count" bytes is read or end of cluster is reached
 * @return  Number of bytes actually read
 */
u32 Fat32Data::read_data_cluster(u32 position, u32 cluster, u8* data, u32 count) const {
    const u16 SECTOR_SIZE_IN_BYTES = bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = sectors_per_cluster * bytes_per_sector;
    u16 byte_in_sector = (position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;
    u8 sector_in_cluster = (position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u32 total_bytes_read = 0;

    for (; sector_in_cluster < sectors_per_cluster; sector_in_cluster++) {
        u16 bytes_in_sector_left = bytes_per_sector - byte_in_sector;
        u16 read_count = min(count, bytes_in_sector_left);

        if (!read_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, data, read_count))
            break;

        count -= read_count;
        total_bytes_read += read_count;
        data += read_count;
        byte_in_sector = 0;

        if (count == 0)
            break;
    }
    return total_bytes_read;
}

/**
 * @brief   Write "size" bytes into given Fat32 data sector
 * @note    IF SIZE < SECTOR SIZE (512 bytes), REMAINING BYTES IN SECTOR ARE FILLED WITH 0 !!!
 */
bool Fat32Data::write_data_sector(u32 cluster, u8 sector_in_cluster, void const* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.write28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_in_cluster, data, size);
}

/**
 * @brief Write "size" bytes of data at [cluster][sector_in_cluster][byte_in_sector]
 */
bool Fat32Data::write_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void const* data, u32 size) const {
    // write across sectors is not supported
    if (byte_in_sector + size > bytes_per_sector)
        return false;

    // write the entire sector, optimal write for FAT32
    if ((byte_in_sector == 0) && (size == bytes_per_sector))
        return write_data_sector(cluster, sector_in_cluster, data, size);

    // write starts somewhere in the middle of sector, need read entire sector first and then update it partially
    u8 buff[bytes_per_sector];
    if (!read_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector))
        return false;

    memcpy(buff + byte_in_sector, data, size);
    return write_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector);
}

/**
 * @brief   Write the cluster starting from "position" byte until "count" bytes is written or end of cluster is reached
 * @return  Number of bytes actually written
 */
u32 Fat32Data::write_data_cluster(u32 position, u32 cluster, const u8* data, u32 count) const {
    const u16 SECTOR_SIZE_IN_BYTES = bytes_per_sector;
    const u16 CLUSTER_SIZE_IN_BYTES = sectors_per_cluster * bytes_per_sector;
    u16 byte_in_sector = (position % CLUSTER_SIZE_IN_BYTES) % SECTOR_SIZE_IN_BYTES;
    u8 sector_in_cluster = (position % CLUSTER_SIZE_IN_BYTES) / SECTOR_SIZE_IN_BYTES;
    u32 total_bytes_written = 0;

    for (; sector_in_cluster < sectors_per_cluster; sector_in_cluster++) {
        u16 bytes_in_sector_left = bytes_per_sector - byte_in_sector;
        u16 written_count = min(count, bytes_in_sector_left);
        if (!write_data_sector_from_byte(cluster, sector_in_cluster, byte_in_sector, data, written_count))
            break;

        count -= written_count;
        total_bytes_written += written_count;
        data += written_count;
        byte_in_sector = 0;

        if (count == 0)
            break;
    }
    return total_bytes_written;
}

void Fat32Data::clear_data_cluster(u32 cluster) const {
    u8 zeroes[bytes_per_sector];
    memset(zeroes, 0, sizeof(zeroes));
    for (u8 sector_offset = 0; sector_offset < sectors_per_cluster; sector_offset++)
        write_data_sector(cluster, sector_offset, zeroes, sizeof(zeroes));
}

/**
 * @brief   Check if we are at the beginning of a cluster
 */
bool Fat32Data::is_cluster_beginning(u32 position) const {
    const u16 CLUSTER_SIZE_IN_BYTES = sectors_per_cluster * bytes_per_sector;
    return (position % CLUSTER_SIZE_IN_BYTES) == 0;
}
} /* namespace fat32 */
} /* namespace filesystem */
