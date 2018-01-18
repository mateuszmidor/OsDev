/**
 *   @file: MassStorageMsDos.cpp
 *
 *   @date: Jul 3, 2017
 * @author: Mateusz Midor
 */

#include "MassStorageMsDos.h"

using drivers::AtaDevice;
using namespace cstd;

namespace filesystem {

MasterBootRecord MassStorageMsDos::read_mbr(const drivers::AtaDevice& hdd) {
    MasterBootRecord mbr;
    hdd.read28(0, &mbr, sizeof(mbr));
    return mbr;
}

bool MassStorageMsDos::verify(const AtaDevice& hdd) {
    MasterBootRecord mbr = read_mbr(hdd);
    return mbr.magic_number == MasterBootRecord::MAGIC_NUMBER;
}

MassStorageMsDos::MassStorageMsDos(const AtaDevice& hdd) {
    // check this is truly MBR formatted device
    if (!verify(hdd))
        return;

    // collect all FAT32 volumes
    MasterBootRecord mbr = read_mbr(hdd);
    for (u8 i = 0; i < 4; i++) {
        const auto& p = mbr.primary_partition[i];
        if (p.partition_id == PARTITION_TYPE_FAT32)
            volumes.emplace_back(hdd, p.bootable, p.start_lba, p.length);
    }
}

vector<VolumeFat32>& MassStorageMsDos::get_volumes() {
    return volumes;
}

} /* namespace filesystem */
