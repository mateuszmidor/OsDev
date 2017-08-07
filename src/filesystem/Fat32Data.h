/**
 *   @file: Fat32Data.h
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32DATA_H_
#define SRC_FILESYSTEM_FAT32DATA_H_

#include "AtaDriver.h"
#include "Fat32Structs.h"

namespace filesystem {


class Fat32Data {
public:
    Fat32Data(drivers::AtaDevice& hdd);
    void setup(u32 data_start, u16 bytes_per_sector, u8 sectors_per_cluster);

    bool read_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const;
    bool read_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void* data, u32 size) const;
    bool write_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const;
    bool write_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void const* data, u32 size) const;
    void clear_data_cluster(u32 cluster) const;

private:
    drivers::AtaDevice& hdd;
    u32 data_start_in_sectors   = 0;
    u16 bytes_per_sector        = 0;
    u8 sectors_per_cluster      = 0;

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32DATA_H_ */
