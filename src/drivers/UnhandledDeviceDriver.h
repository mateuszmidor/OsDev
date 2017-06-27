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
    s16 handled_interrupt_no() override;
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) override;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_UNHANDLEDDEVICEDRIVER_H_ */
