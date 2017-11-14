/**
 *   @file: PitDriver.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "PitDriver.h"

using namespace hardware;

namespace drivers {

s16 PitDriver::handled_interrupt_no() {
    return Interrupts::PIT;
}

hardware::CpuState* PitDriver::on_interrupt(hardware::CpuState* cpu_state) {
    return on_tick(cpu_state);
}

void PitDriver::set_channel0_on_tick(const OnTickEvent &event) {
    on_tick = event;
}

/**
 * @brief   Set PIT channel0 interrupt frequency
 * @param   hz Frequency in range 18 - 1'193'182 [Hz]
 * @ref     http://wiki.osdev.org/Programmable_Interval_Timer#Channel_0
 *          http://kernelx.weebly.com/programmable-interval-timer.html
 */
void PitDriver::set_channel0_hz(u16 hz) {
    const u8 MODE       = 0x36;         // channel 0 + lobyte/hibyte access mode + square wave generator + 16-bit binary mode
    const u32 DIVISOR   = PIT_OSCILLATOR_HZ / hz;

    this->channel0_hz = hz;
    pit_cmd.write(MODE);
    pit_channel_0.write(DIVISOR & 0xFF);
    pit_channel_0.write(DIVISOR >> 8);
}

/**
 * @brief   Get PIT interrupt frequency
 */
u16 PitDriver::get_channel0_hz() const {
    return channel0_hz;
}

/**
 * @brief   Set PIT channel2 countdown counter value. It will decrement at the rate of 1193180 Hz
 * @param   count Value from which the countdown starts, maximum is 65536 (0x10000)
 */
void PitDriver::set_channel2_count(u32 count) {
    const u8 MODE = 0xB4;    // channel 2 + lobyte/hibyte access mode  + rate generator  + 16-bit binary mode

    u16 real_count = count >= 0x10000 ? 0 : count; // for PIT, 0 means 0x10000 (maximum supported value)

    pit_cmd.write(MODE);
    pit_channel_2.write(real_count & 0xFF);
    pit_channel_2.write(real_count >> 8);
}

/**
 * @rbrief  Get PIT channel2 current countdown counter value
 */
u16 PitDriver::get_channel2_count() const {
    u8 lo = pit_channel_2.read();
    u8 hi = pit_channel_2.read();
    return (hi << 8) + lo;
}

} /* namespace drivers */
