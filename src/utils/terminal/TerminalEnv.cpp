/**
 *   @file: TerminalEnv.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "TerminalEnv.h"
#include "kstd.h"
#include "VolumeFat32.h"
#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;
namespace terminal {

TerminalEnv::TerminalEnv() :
    printer(nullptr) {

    auto& driver_manager = DriverManager::instance();
    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus)
        return;

    if (ata_primary_bus->master_hdd.is_present()) {
        install_volumes(ata_primary_bus->master_hdd);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        install_volumes(ata_primary_bus->slave_hdd);
    }

    if (!volumes.empty()) {
        cwd = "/";
        volume = &volumes[0];
    }
    else {
        cwd = "[NO VOLUMES]";
        volume = nullptr;
    }
}

void TerminalEnv::install_volumes(AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd))
        return;

    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes())
        volumes.push_back(v);
}
} /* namespace terminal */
