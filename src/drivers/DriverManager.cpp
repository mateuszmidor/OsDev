/**
 *   @file: DriverManager.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "DriverManager.h"

namespace drivers {

DriverManager DriverManager::_instance;

DriverManager& DriverManager::instance() {
    return _instance;
}

DriverManager::~DriverManager() {
    for (auto d : drivers)
        if (d)
            delete d;
}

DriverManager::DriverManager() {
}

/**
 * @brief   This method takes ownership and of pointer drv
 */
void DriverManager::install_driver(DeviceDriver *drv, u8 interrupt_no) {
    drivers[interrupt_no] = drv;
}

void DriverManager::on_interrupt(u8 interrupt_no) const {
    if (auto d = drivers[interrupt_no])
        d->on_interrupt();
}
} /* namespace drivers */
