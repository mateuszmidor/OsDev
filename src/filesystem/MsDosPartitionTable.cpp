/**
 *   @file: MsDosPartitionTable.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include "MsDosPartitionTable.h"
#include "ScreenPrinter.h"

using drivers::AtaDriver;

namespace filesystem {

MsDosPartitionTable::MsDosPartitionTable() {
}

MsDosPartitionTable::~MsDosPartitionTable() {
}

void MsDosPartitionTable::read_partitions(AtaDriver* hd) {
    ScreenPrinter& printer = ScreenPrinter::instance();

    MasterBotRecord mbr;
    hd->read28(0, &mbr, sizeof(mbr));

    if (mbr.magic_number != 0xAA55)
        printer.format("MBR mgic number % != %\n", mbr.magic_number, 0xAA55);

    for (u8 i = 0; i < 4; i++) {
        auto &pte = mbr.primary_partition[i];

        printer.format("Parition %\n", i);

        printer.format("bootable %, partition_id %, start_lba %, length %\n",
                pte.bootable, pte.partition_id, pte.start_lba, pte.length);

        printer.format("START head %, cylinder %, sector %, END head % cylinder % sector %\n\n",
                pte.start_head, pte.start_cylinder, pte.start_sector,
                pte.end_head, pte.end_cylinder, pte.end_sector);
    }
}

} /* namespace filesystem */
