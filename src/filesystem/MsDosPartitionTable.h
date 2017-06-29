/**
 *   @file: MsDosPartitionTable.h
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_
#define SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_

#include "AtaDriver.h"

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

    u32 start_lba;
    u32 length;
} __attribute__((packed));

struct MasterBotRecord {
    u8  bootloader[440];
    u32 signature;
    u16 reserved;
    PartitionTableEntry primary_partition[4];
    u16 magic_number;
} __attribute__((packed));

class MsDosPartitionTable {
public:
    MsDosPartitionTable();
    virtual ~MsDosPartitionTable();
    static void read_partitions(drivers::AtaDevice& hd);
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_MSDOSPARTITIONTABLE_H_ */
