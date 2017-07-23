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
    return Interrupts::Timer;
}

cpu::CpuState* TimerDriver::on_interrupt(cpu::CpuState* cpu_state) {
    return on_tick(cpu_state);
}

void TimerDriver::set_on_tick(const TimerEvent &event) {
    on_tick = event;
}

/**
 * @brief   Set PIC interrupt frequency
 * @param   hz Frequency in range 18 - 1'193'182 [Hz]
 * @ref     http://wiki.osdev.org/Programmable_Interval_Timer#Channel_0
 *          http://kernelx.weebly.com/programmable-interval-timer.html
 */
void TimerDriver::set_hz(u16 hz) {
    const u8 MODE       = 0x36; // channel 0 + lobyte/hibyte access + square wave generator + 16-bit binary mode
    const u32 DIVISOR   = 1193180 / hz;
    pit_cmd.write(MODE);
    pit_channel_0.write(DIVISOR & 0xFF);
    pit_channel_0.write(DIVISOR >> 8);
}

} /* namespace drivers */
