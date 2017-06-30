/**
 *   @file: MsDosPartitionTable.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include "MsDosPartitionTable.h"
#include "kstd.h"

using drivers::AtaDevice;
using kstd::string;
using kstd::enum_to_str;

namespace filesystem {

MasterBootRecord MsDosPartitionTable::read_mbr(AtaDevice& hd) {
    MasterBootRecord mbr;
    hd.read28(0, &mbr, sizeof(mbr));
    return mbr;
}

void MsDosPartitionTable::print_mbr(const MasterBootRecord& mbr, ScreenPrinter& printer) {
    if (mbr.magic_number != MBR_MAGIC)
        printer.format("MBR mgic number % != %\n", mbr.magic_number, MBR_MAGIC);

    for (u8 i = 0; i < 4; i++) {
        auto& pte = mbr.primary_partition[i];
        printer.format("Parition %\n", i);

        string bootable = enum_to_str(pte.bootable,
                "false=0x0",
                "true=0x80"
                );

        string partition_id = enum_to_str(pte.partition_id,
                "EMPTY=0x00",
                "WIN95 FAT32=0x0B",
                "LINUX NATIVE=0x83"
                );

        printer.format("bootable %, partition_id %, start_lba %, length %\n", bootable.c_str(), partition_id.c_str(), pte.start_lba, pte.length);
        printer.format("START head %, cylinder %, sector %, END head % cylinder % sector %\n\n", pte.start_head, pte.start_cylinder, pte.start_sector,
                pte.end_head, pte.end_cylinder, pte.end_sector);
    }
}
} /* namespace filesystem */
