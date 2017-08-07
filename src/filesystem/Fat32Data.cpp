/**
 *   @file: Fat32Data.cpp
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Data.h"
#include "kstd.h"

namespace filesystem {

Fat32Data::Fat32Data(drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Data::setup(u32 data_start_in_sectors, u16 bytes_per_sector, u8 sectors_per_cluster) {
    this->data_start_in_sectors = data_start_in_sectors;
    this->bytes_per_sector = bytes_per_sector;
    this->sectors_per_cluster = sectors_per_cluster;
}

bool Fat32Data::read_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.read28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
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

bool Fat32Data::write_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.write28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
}

/**
 * @brief Write "size" bytes of data at [cluster][sector_in_cluster][byte_in_sector]
 */
bool Fat32Data::write_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void const* data, u32 size) const {
    // write across sectors is not supported
    if (byte_in_sector + size > bytes_per_sector)
        return false;

    // write starts at sector beginning, optimal write
    if (byte_in_sector == 0)
        return write_data_sector(cluster, sector_in_cluster, data, size);

    // write starts somewhere in the middle of sector, need read entire sector first and then update it partially
    u8 buff[bytes_per_sector];
    if (!read_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector))
        return false;

    memcpy(buff + byte_in_sector, data, size);
    return write_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector);
}

void Fat32Data::clear_data_cluster(u32 cluster) const {
    u8 zeroes[bytes_per_sector];
    memset(zeroes, 0, sizeof(zeroes));
    for (u8 sector_offset = 0; sector_offset < sectors_per_cluster; sector_offset++)
        write_data_sector(cluster, sector_offset, zeroes, sizeof(zeroes));
}

} /* namespace filesystem */
