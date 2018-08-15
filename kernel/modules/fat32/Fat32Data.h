/**
 *   @file: Fat32Data.h
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_FAT32DATA_H_
#define SRC_FILESYSTEM_FAT32_FAT32DATA_H_

#include "AtaDriver.h"
#include "Fat32Structs.h"

namespace filesystem {

/**
 * @brief   Fat32 volume data. It knows about sectors and clusters but not about cluster chains so read/write cant cross cluster boundary.
 */
class Fat32Data {
public:
    Fat32Data(const drivers::AtaDevice& hdd);
    void setup(u32 data_start, u16 bytes_per_sector, u8 sectors_per_cluster);

    bool read_data_sector(u32 cluster, u8 sector_in_cluster, void* data, u32 size) const;
    bool read_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void* data, u32 size) const;
    u32 read_data_cluster(u32 position, u32 cluster, u8* data, u32 count) const;
    bool write_data_sector(u32 cluster, u8 sector_in_cluster, void const* data, u32 size) const;
    bool write_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void const* data, u32 size) const;
    u32 write_data_cluster(u32 position, u32 cluster, const u8* data, u32 count) const;
    void clear_data_cluster(u32 cluster) const;
    bool is_cluster_beginning(u32 position) const;
    u16 get_bytes_per_sector() const { return bytes_per_sector; }
    u8 get_sectors_per_cluster() const { return sectors_per_cluster; }

private:
    const drivers::AtaDevice&   hdd;
    u32                         data_start_in_sectors   = 0;
    u16                         bytes_per_sector        = 0;
    u8                          sectors_per_cluster     = 0;

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32_FAT32DATA_H_ */
