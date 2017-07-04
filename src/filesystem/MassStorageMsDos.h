/**
 *   @file: MassStorageMsDos.h
 *
 *   @date: Jul 3, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_MASSSTORAGEMSDOS_H_
#define SRC_FILESYSTEM_MASSSTORAGEMSDOS_H_

#include "kstd.h"
#include "AtaDriver.h"
#include "VolumeFat32.h"
#include "ScreenPrinter.h"

namespace filesystem {

struct PartitionTableEntry {
    u8  bootable;

    u8  start_head;
    u8  start_sector    : 6;
    u16 start_cylinder  :10;

    u8  partition_id;   // 0x00 for no partition, 0x0B/0x0C for FAT32, 0x82 and 0x83 for linux, see https://www.win.tue.nl/~aeb/partitions/partition_types-1.html

    u8  end_head;
    u8  end_sector      : 6;
    u16 end_cylinder    :10;

    u32 start_lba;      // partition offset (first sector number)
    u32 length;         // partition size in sectors
} __attribute__((packed));

struct MasterBootRecord {
    u8  bootloader[440];
    u32 signature;
    u16 reserved;
    PartitionTableEntry primary_partition[4];
    u16 magic_number;

    static const u16 MAGIC_NUMBER = 0xAA55;
} __attribute__((packed));

/**
 * @brief   Represents mass storage device with MsDos Partition Table (MBR at sector 0).
 *          Such device can contain partitions of any type, especially Fat32 :)
 */
class MassStorageMsDos {
public:
    static bool verify(drivers::AtaDevice& hdd);
    MassStorageMsDos(drivers::AtaDevice& hdd);
    kstd::vector<VolumeFat32>& get_volumes();

private:
    static MasterBootRecord read_mbr(drivers::AtaDevice& hdd);

    // see https://www.win.tue.nl/~aeb/partitions/partition_types-1.html
    static const u8 PARTITION_TYPE_NONE     = 0x00;
    static const u8 PARTITION_TYPE_FAT32    = 0x0B;

    kstd::vector<VolumeFat32> volumes;  // in the future should be of type: shared_ptr<GenericVolume>
};

} /* namespace cpuexceptions */

#endif /* SRC_FILESYSTEM_MASSSTORAGEMSDOS_H_ */
