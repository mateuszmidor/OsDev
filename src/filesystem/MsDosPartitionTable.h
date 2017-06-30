/**
 *   @file: MsDosPartitionTable.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_
#define SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_

#include "AtaDriver.h"
#include "ScreenPrinter.h"

namespace filesystem {

struct PartitionTableEntry {
    u8  bootable;

    u8  start_head;
    u8  start_sector    : 6;
    u16 start_cylinder  :10;

    u8  partition_id;   // 0x00 for no partition, 0x0B for windows, 0x83 for linux, see https://www.win.tue.nl/~aeb/partitions/partition_types-1.html

    u8  end_head;
    u8  end_sector      : 6;
    u16 end_cylinder    :10;

    u32 start_lba;      // partition offset (first sector number)
    u32 length;
} __attribute__((packed));

struct MasterBootRecord {
    u8  bootloader[440];
    u32 signature;
    u16 reserved;
    PartitionTableEntry primary_partition[4];
    u16 magic_number;   // must be MBR_MAGIC(0xAA55) for valid MBR
} __attribute__((packed));

class MsDosPartitionTable {
public:
    static MasterBootRecord read_mbr(drivers::AtaDevice& hd);
    static void print_mbr(const MasterBootRecord& mbr, ScreenPrinter& printer);

    static const int MBR_MAGIC = 0xAA55;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_ */
