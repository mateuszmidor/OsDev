/**
 *   @file: TimerDriver.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "TimerDriver.h"

using namespace cpu;

namespace drivers {

s16 TimerDriver::handled_interrupt_no() {
    return 32;
}

cpu::CpuState* TimerDriver::on_interrupt(cpu::CpuState* cpu_state) {
    return on_tick(cpu_state);
}

void TimerDriver::set_on_tick(const TimerEvent &event) {
    on_tick = event;
}
} /* namespace drivers */
