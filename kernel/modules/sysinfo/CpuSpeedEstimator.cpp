/*
 * CpuSpeedEstimator.cpp
 *
 *  Created on: Dec 21, 2018
 *      Author: Mateusz Midor
 */

#include "CpuSpeedEstimator.h"
#include "CpuInfo.h"
#include "PitDriver.h"
#include "DriverManager.h"
#include "InterruptManager.h"

namespace sysinfo {
namespace cpuspeedestimator {

/**
 * @brief   Determine approximate peak CPU speed in MHz
 * @see     http://wiki.osdev.org/Detecting_CPU_Speed#Without_Interrupts
 */
cstd::Optional<u32> estimate_peak_mhz() {
    const u32 COUNTER_MAX = 0x10000;

    auto& driver_manager = drivers::DriverManager::instance();
    auto pit = driver_manager.get_driver<drivers::PitDriver>();

    if (!pit)
        return {"cpuspeedestimator::estimate_peak_mhz: no PitDriver\n"};

    // disable interrupts
    auto& interrupt_manager = hardware::InterruptManager::instance();
    u16 interrupt_mask = interrupt_manager.disable_interrupts();

    // set PIT countdown counter to maximum (0x10000)
    pit->set_channel2_count(COUNTER_MAX);

    // do busy loop and count number of CPU cycles used
    hardware::CpuInfo cpu;
    u64 start_cycles = cpu.get_rtdsc();
    for (u32 i = 0; i < 1 * 1000 * 1000; i++)   // 1'000'000 iterations was tested for i5 3.2GHz
        asm volatile("xor %rax, %rdx;"); // xor takes 1 CPU cycle, volatile ensures it doesn't get optimized away along with the entire loop :)
    u64 stop_cycles = cpu.get_rtdsc();

    // read PIT countdown counter current value
    u64 ticks = COUNTER_MAX - pit->get_channel2_count();

    // cpu hz = cpu_cycles / time
    u64 cpu_hz = (stop_cycles - start_cycles) * drivers::PitDriver::PIT_OSCILLATOR_HZ / ticks;

    // restore interrupts
    interrupt_manager.enable_interrupts(interrupt_mask);

    return cpu_hz  / 1000 / 1000;
}

} /* namespace cpuspeedestimator */
} /* namespace sysinfo */
