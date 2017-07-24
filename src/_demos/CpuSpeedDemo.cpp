/**
 *   @file: CpuSpeedDemo.cpp
 *
 *   @date: Jul 24, 2017
 * @author: Mateusz Midor
 */

#include "CpuSpeedDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "PitDriver.h"

using namespace drivers;
using namespace multitasking;
using utils::KernelLog;

namespace demos {

void CpuSpeedDemo::run() {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();
    auto& task_manager = TaskManager::instance();
    auto pit = driver_manager.get_driver<PitDriver>();

    if (!pit) {
        klog.format("CpuSpeedDemo::run: no PitDriver\n");
        return;
    }

    u32 num_loops = pit->get_hz();
    u32 num_tasks = 0;

    u64 start_in_cycles = rtdsc();
    for (u32 i = 0; i < num_loops; i++ ) {
        num_tasks += task_manager.get_num_tasks();
        Task::yield();
    }
    u64 stop_in_cycles = rtdsc();

    u64 cycles = stop_in_cycles - start_in_cycles;
    klog.format("\nCPU speed: %MHz\n", cycles * num_loops / num_tasks / 1000/ 1000);
}

u64 CpuSpeedDemo::rtdsc() {
    u32 hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)lo | ((u64)hi << 32));
}

} /* namespace demos */
