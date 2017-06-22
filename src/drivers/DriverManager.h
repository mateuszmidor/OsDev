/**
 *   @file: DriverManager.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_DRIVERMANAGER_H_
#define SRC_DRIVERS_DRIVERMANAGER_H_

#include <array>
#include <memory>
#include "DeviceDriver.h"

namespace drivers {

using DeviceDriverPtr = std::shared_ptr<DeviceDriver>;

class DriverManager {
public:
    static DriverManager& instance();
    virtual ~DriverManager();
    void install_driver(DeviceDriverPtr drv);
    void on_interrupt(u8 interrupt_no) const;

private:
    static DriverManager _instance;

    DriverManager();

    std::array<DeviceDriverPtr, 256> drivers; // this array maps interrupt_no to handling driver
    u8 driver_count = 0;

};

} /* namespace drivers */

#endif /* SRC_DRIVERS_DRIVERMANAGER_H_ */
