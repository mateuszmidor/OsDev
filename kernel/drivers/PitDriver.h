/**
 *   @file: PitDriver.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_PITDRIVER_H_
#define SRC_DRIVERS_PITDRIVER_H_

#include <functional>
#include "DeviceDriver.h"

namespace drivers {

// OnTickEvent; it is focused on task switching
using OnTickEvent = std::function<hardware::CpuState*(hardware::CpuState* cpu_state)>;

/**
 * @brief   This is a driver for Programmable Interval Timer (PIT) that is normally used for scheduling processes
 */
class PitDriver: public DeviceDriver {
public:
    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;
    void set_channel0_on_tick(const OnTickEvent &event);
    void set_channel0_hz(u16 hz);
    u16 get_channel0_hz() const;
    void set_channel2_count(u32 count);
    u16 get_channel2_count() const;

    static const u32 PIT_OSCILLATOR_HZ  {1193180};

private:
    u16 channel0_hz                     {18};   // default Programmable Interval Timer frequency in Hz
    hardware::Port8bit pit_channel_0    {0x40};
    hardware::Port8bit pit_channel_1    {0x41};
    hardware::Port8bit pit_channel_2    {0x42};
    hardware::Port8bit pit_cmd          {0x43};
    OnTickEvent on_tick = [](hardware::CpuState* cpu_state) { return cpu_state; };
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_PITDRIVER_H_ */
