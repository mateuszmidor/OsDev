/**
 *   @file: DriverManager.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_DRIVERMANAGER_H_
#define SRC_DRIVERS_DRIVERMANAGER_H_

#include <array>
#include "CpuState.h"
#include "UnhandledDeviceDriver.h"

namespace drivers {

using DeviceDriverPtr = DeviceDriver*;

class DriverManager {
public:
    static DriverManager& instance();
    DriverManager operator=(const DriverManager&) = delete;
    DriverManager operator=(DriverManager&&) = delete;

    virtual ~DriverManager();

    template <class DrvType>
    void install_driver(DrvType* drv) {
        auto interrupt_no = DrvType::handled_interrupt_no();
        drivers[interrupt_no] = drv;
    }

    template <class DrvType>
    DrvType* get_driver() {
        auto interrupt_no = DrvType::handled_interrupt_no();
        return (DrvType*)drivers[interrupt_no];
    }

    hardware::CpuState* on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state) const;

private:
    static DriverManager _instance;
    static UnhandledDeviceDriver unhandled_device_driver;

    DriverManager();

    std::array<DeviceDriverPtr, 256> drivers; // this array maps interrupt_no to handling driver
    u8 driver_count = 0;

};

} /* namespace drivers */

#endif /* SRC_DRIVERS_DRIVERMANAGER_H_ */
