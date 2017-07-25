/**
 *   @file: CpuSpeedDemo.cpp
 *
 *   @date: Jul 24, 2017
 * @author: Mateusz Midor
 */

#include "CpuSpeedDemo.h"
#include "KernelLog.h"
#include "InterruptManager.h"
#include "DriverManager.h"
#include "PitDriver.h"
#include "Port.h"

using namespace drivers;
using namespace multitasking;
using utils::KernelLog;

namespace demos {

void CpuSpeedDemo::run() {
    no_interrupt_method();
//    interrupt_method();
}

/**
 * @ref http://wiki.osdev.org/Detecting_CPU_Speed#Without_Interrupts
 */
void CpuSpeedDemo::no_interrupt_method() {
    const u32 COUNTER_MAX = 0x10000;

    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();
    auto& task_manager = TaskManager::instance();
    auto pit = driver_manager.get_driver<PitDriver>();

    if (!pit) {
        klog.format("CpuSpeedDemo::run: no PitDriver\n");
        return;
    }

    // disable interrupts
    auto& interrupt_manager = hardware::InterruptManager::instance();
    u16 interrupt_mask = interrupt_manager.disable_interrupts();

    // set PIT countdown counter to maximum (0x10000)
    pit->set_channel2_count(COUNTER_MAX);

    // do busy loop and count number of CPU cycles used
    u64 start_cycles = rtdsc();
    for (u32 i = 0; i < 1 * 1000 * 1000; i++)   // 1'000'000 iterations was tested for i5 3.2GHz
        asm volatile("xor %rax, %rdx;"); // xor takes 1 CPU cycle, volatile ensures it doesn't get optimized away along with the entire loop :)
    u64 stop_cycles = rtdsc();

    // read PIT countdown counter current value
    u64 ticks = COUNTER_MAX - pit->get_channel2_count();

    // cpu hz = cpu_cycles / time
    u64 cpu_hz = (stop_cycles - start_cycles) * PitDriver::PIT_OSCILLATOR_HZ / ticks;

    // restore interrupts
    interrupt_manager.enable_interrupts(interrupt_mask);

    klog.format("CPU speed: %MHz\n", cpu_hz  / 1000 / 1000);
}

void CpuSpeedDemo::interrupt_method() {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();
    auto& task_manager = TaskManager::instance();
    auto pit = driver_manager.get_driver<PitDriver>();

    if (!pit) {
        klog.format("CpuSpeedDemo::run: no PitDriver\n");
        return;
    }

    u32 num_loops = pit->get_channel0_hz();
    u32 num_tasks = 0;

    u64 start_in_cycles = rtdsc();
    for (u32 i = 0; i < num_loops; i++) {
        num_tasks += task_manager.get_num_tasks();
        Task::yield();
    }

    u64 stop_in_cycles = rtdsc();
    u64 cycles = stop_in_cycles - start_in_cycles;
    klog.format("CPU speed: %MHz\n", cycles * num_loops / num_tasks / 1000 / 1000);
}

u64 CpuSpeedDemo::rtdsc() {
    u32 hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)lo | ((u64)hi << 32));
}

} /* namespace demos */
