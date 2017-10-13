/**
 *   @file: DriverManager.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "DriverManager.h"

using namespace hardware;

namespace drivers {

DriverManager DriverManager::_instance;
UnhandledDeviceDriver DriverManager::unhandled_device_driver(0xFF); // TODO: handle unhandled devices some better way

DriverManager& DriverManager::instance() {
    return _instance;
}

DriverManager::~DriverManager() {
}

DriverManager::DriverManager() {
    for (int i = 0; i < drivers.size(); i++)
        drivers[i] = &unhandled_device_driver;
}

CpuState* DriverManager::on_interrupt(u8 interrupt_no, CpuState* cpu_state) const {
    return drivers[interrupt_no]->on_interrupt(cpu_state);
}
} /* namespace drivers */
