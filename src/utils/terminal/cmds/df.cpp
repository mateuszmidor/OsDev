/**
 *   @file: df.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "df.h"
#include "VolumeFat32.h"
#include "kstd.h"
#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;

namespace cmds {

void df::run() {
    auto& driver_manager = DriverManager::instance();
    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus)
        return;

    if (ata_primary_bus->master_hdd.is_present()) {
        env->printer->format("ATA Primary Master:\n");
        print_hdd_info(ata_primary_bus->master_hdd);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        env->printer->format("ATA Primary Slave:\n");
        print_hdd_info(ata_primary_bus->slave_hdd);
    }
}

void df::print_hdd_info(drivers::AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd)) {
        env->printer->format("Not MBR formatted device\n");
        return;
    }

    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        u32 size_in_bytes = v.get_size_in_bytes();
        u32 used_space_in_bytes = v.get_used_space_in_bytes();
        u32 cluster_in_bytes = v.get_cluster_size_in_bytes();
        env->printer->format("  %, size: %MB, used: %KB (% clusters), cluster size: %B\n",
                v.get_label(),
                size_in_bytes / 1024 / 1024,
                used_space_in_bytes / 1024,
                used_space_in_bytes / cluster_in_bytes,
                cluster_in_bytes);
    }
}
} /* namespace cmds */
