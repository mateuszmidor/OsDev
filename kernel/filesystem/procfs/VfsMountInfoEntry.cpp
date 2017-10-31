/**
 *   @file: VfsMountInfoEntry.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include "VfsMountInfoEntry.h"
#include "DriverManager.h"
#include "fat32/MassStorageMsDos.h"
#include <errno.h>

using namespace drivers;
using namespace filesystem;
namespace filesystem {

bool VfsMountInfoEntry::open() {
    if (is_open)
        return false;

    is_open = true;
    return true;
}

void VfsMountInfoEntry::close() {
    is_open = false;
}

/**
 * @brief   The size of memory info is not known until the info string is built
 */
u32 VfsMountInfoEntry::get_size() const {
    return 0;
}

/**
 * @brief   Read the last "count" of info bytes
 * @return  Num of read bytes
 */
s64 VfsMountInfoEntry::read(void* data, u32 count) {
    if (!is_open) {
        return 0;
    }

    if (count == 0)
        return 0;


    const kstd::string info = get_info();
    if (info.empty())
        return 0;

    u32 read_start = kstd::max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = kstd::min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close();
    return num_bytes_to_read;
}

kstd::string VfsMountInfoEntry::get_info() const {
    kstd::string result;
    auto& driver_manager = DriverManager::instance();

    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();

    if (ata_primary_bus->master_hdd.is_present()) {
        result += "ATA Primary Master:\n";
        result += get_hdd_info(ata_primary_bus->master_hdd);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        result += "ATA Primary Slave:\n";
        result += get_hdd_info(ata_primary_bus->slave_hdd);
    }


    auto ata_secondary_bus = driver_manager.get_driver<AtaSecondaryBusDriver>();

    if (ata_secondary_bus->master_hdd.is_present()) {
        result += "ATA Secondary Master:\n";
        result += get_hdd_info(ata_primary_bus->master_hdd);
    }

    if (ata_secondary_bus->slave_hdd.is_present()) {
        result += "ATA Secondary Slave:\n";
        result += get_hdd_info(ata_primary_bus->slave_hdd);
    }

    return result;
}

kstd::string VfsMountInfoEntry::get_hdd_info(drivers::AtaDevice& hdd) const {
    if (!MassStorageMsDos::verify(hdd)) {
        return "Not MBR formatted device\n";
    }

    kstd::string result;
    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        u32 size_in_bytes = v.get_size_in_bytes();
        u32 used_space_in_bytes = v.get_used_space_in_bytes();
        u32 cluster_in_bytes = v.get_cluster_size_in_bytes();
        result += format("  %, size: %MB, used: %KB (% clusters), cluster size: %B\n",
                v.get_label(),
                size_in_bytes / 1024 / 1024,
                used_space_in_bytes / 1024,
                used_space_in_bytes / cluster_in_bytes,
                cluster_in_bytes);
    }

    return result;
}

s64 VfsMountInfoEntry::write(const void* data, u32 count) {
    return -EPERM;
}

} /* namespace filesystem */
