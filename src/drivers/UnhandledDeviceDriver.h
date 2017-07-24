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
    UnhandledDeviceDriver(u8 interrupt_no);
    ~UnhandledDeviceDriver() override {}
    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;

private:
    u8 interrupt_no;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_UNHANDLEDDEVICEDRIVER_H_ */
