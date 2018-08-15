/**
 *   @file: VfsMountInfoEntry.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "kstd.h"
#include "DriverManager.h"
#include "VfsMountInfoEntry.h"
#include "MassStorageMsDos.h"

using namespace cstd;
using namespace drivers;
using namespace filesystem;

namespace filesystem {

utils::SyscallResult<EntryState*> VfsMountInfoEntry::open() {
    if (is_open)
        return {middlespace::ErrorCode::EC_AGAIN};

    is_open = true;
    return {nullptr};
}

utils::SyscallResult<void> VfsMountInfoEntry::close(EntryState*) {
    is_open = false;
    return {middlespace::ErrorCode::EC_OK};
}

/**
 * @brief   Read the last "count" bytes of mount info string
 * @return  Num of read bytes
 */
utils::SyscallResult<u64> VfsMountInfoEntry::read(EntryState*, void* data, u32 count) {
    if (!is_open)
        return {0};

    if (count == 0)
        return {0};

    const string info = get_info();
    if (info.empty())
        return {0};

    u32 read_start = max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close(nullptr);
    return {num_bytes_to_read};
}

string VfsMountInfoEntry::get_info() const {
    string result;
    auto& driver_manager = DriverManager::instance();

    if (auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>()) {
        if (ata_primary_bus->master_hdd.is_present()) {
            result += "ATA Primary Master:\n";
            result += get_hdd_info(ata_primary_bus->master_hdd);
        }

        if (ata_primary_bus->slave_hdd.is_present()) {
            result += "ATA Primary Slave:\n";
            result += get_hdd_info(ata_primary_bus->slave_hdd);
        }
    }

    if (auto ata_secondary_bus = driver_manager.get_driver<AtaSecondaryBusDriver>()) {
        if (ata_secondary_bus->master_hdd.is_present()) {
            result += "ATA Secondary Master:\n";
            result += get_hdd_info(ata_secondary_bus->master_hdd);
        }

        if (ata_secondary_bus->slave_hdd.is_present()) {
            result += "ATA Secondary Slave:\n";
            result += get_hdd_info(ata_secondary_bus->slave_hdd);
        }
    }

    return result;
}

string VfsMountInfoEntry::get_hdd_info(drivers::AtaDevice& hdd) const {
    if (!MassStorageMsDos::verify(hdd)) {
        return "Not MBR formatted device\n";
    }

    string result;
    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        u32 size_in_bytes = v.get_size_in_bytes();
        u32 used_space_in_bytes = v.get_used_space_in_bytes();
        u32 cluster_in_bytes = v.get_cluster_size_in_bytes();
        result += StringUtils::format("  %, size: %MB, used: %KB (% clusters), cluster size: %B\n",
                                        v.get_label(),
                                        size_in_bytes / 1024 / 1024,
                                        used_space_in_bytes / 1024,
                                        used_space_in_bytes / cluster_in_bytes,
                                        cluster_in_bytes);
    }

    return result;
}

} /* namespace filesystem */
