/**
 *   @file: SysCallDriver.h
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_SYSCALLDRIVER_H_
#define SRC_DRIVERS_SYSCALLDRIVER_H_

#include "DeviceDriver.h"

namespace drivers {

class SysCallDriver: public DeviceDriver {
public:
    SysCallDriver() {}
    virtual ~SysCallDriver() {}

    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_SYSCALLDRIVER_H_ */
