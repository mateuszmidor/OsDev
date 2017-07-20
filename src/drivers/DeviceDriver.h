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
    DeviceDriver();
    virtual ~DeviceDriver();

    // number of interrupt in idt that this driver handles
    virtual s16 handled_interrupt_no() = 0;

    // if no task switching to be done, we should just return cpu_state
    virtual cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) = 0;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_DEVICEDRIVER_H_ */
