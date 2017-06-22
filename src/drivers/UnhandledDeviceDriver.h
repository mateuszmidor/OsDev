/**
 *   @file: UnhandledDeviceDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_UNHANDLEDDEVICEDRIVER_H_
#define SRC_DRIVERS_UNHANDLEDDEVICEDRIVER_H_

#include "DeviceDriver.h"

namespace drivers {

class UnhandledDeviceDriver: public DeviceDriver {
public:
    UnhandledDeviceDriver();
    virtual ~UnhandledDeviceDriver();

    s16 handled_interrupt_no() override;
    void on_interrupt() override;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_UNHANDLEDDEVICEDRIVER_H_ */
