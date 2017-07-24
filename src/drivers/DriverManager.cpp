/**
 *   @file: DriverManager.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "DriverManager.h"
#include "UnhandledDeviceDriver.h"

using namespace hardware;

namespace drivers {

DriverManager DriverManager::_instance;

DriverManager& DriverManager::instance() {
    return _instance;
}

DriverManager::~DriverManager() {
}

DriverManager::DriverManager() {
    for (int i = 0; i < drivers.size(); i++)
        drivers[i] = std::make_shared<UnhandledDeviceDriver>(i);
}

CpuState* DriverManager::on_interrupt(u8 interrupt_no, CpuState* cpu_state) const {
    return drivers[interrupt_no]->on_interrupt(cpu_state);
}
} /* namespace drivers */
