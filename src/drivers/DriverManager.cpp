/**
 *   @file: DriverManager.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "DriverManager.h"
#include "UnhandledDeviceDriver.h"

using namespace cpu;

namespace drivers {

DriverManager DriverManager::_instance;

DriverManager& DriverManager::instance() {
    return _instance;
}

DriverManager::~DriverManager() {
}

DriverManager::DriverManager() {
    DeviceDriverPtr unhandled_device_driver = std::make_shared<UnhandledDeviceDriver>();
    for (int i = 0; i < drivers.size(); i++)
        drivers[i] = unhandled_device_driver;
}

void DriverManager::install_driver(DeviceDriverPtr drv) {
    auto interrupt_no = drv->handled_interrupt_no();
    drivers[interrupt_no] = drv;
}

CpuState* DriverManager::on_interrupt(u8 interrupt_no, CpuState* cpu_state) const {
    return drivers[interrupt_no]->on_interrupt(cpu_state);
}
} /* namespace drivers */
