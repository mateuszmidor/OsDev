/**
 *   @file: Driver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_DEVICEDRIVER_H_
#define SRC_DRIVERS_DEVICEDRIVER_H_

#include "Port.h"
#include "InterruptNumbers.h"
#include "CpuState.h"

namespace drivers {

class DeviceDriver {
public:
    DeviceDriver() {}
    virtual ~DeviceDriver() {}

    // DeviceDriver descendants must implement following static function:
    // static s16 handled_interrupt_no();

    // if no task switching to be done, we should just return cpu_state
    virtual hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) = 0;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_DEVICEDRIVER_H_ */
