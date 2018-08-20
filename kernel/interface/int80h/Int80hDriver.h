/**
 *   @file: Int80hDriver.h
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_INT80HDRIVER_H_
#define SRC_DRIVERS_INT80HDRIVER_H_

#include "DeviceDriver.h"

namespace drivers {

class Int80hDriver: public DeviceDriver {
public:
    Int80hDriver() {}
    virtual ~Int80hDriver() {}

    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;

private:
    hardware::CpuState* task_exit(u64 exit_code);
    hardware::CpuState* task_exit_group(u64 exit_code);
    hardware::CpuState* task_kill(hardware::CpuState* cpu_state, u32 task_id);
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_INT80HDRIVER_H_ */
