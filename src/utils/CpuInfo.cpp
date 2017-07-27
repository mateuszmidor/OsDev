/**
 *   @file: CpuInfo.cpp
 *
 *   @date: Jun 7, 2017
 * @author: mateusz
 */

#include "CpuInfo.h"
#include "KernelLog.h"
#include "PitDriver.h"
#include "DriverManager.h"
#include "InterruptManager.h"

using namespace kstd;
using namespace drivers;
namespace utils {

void CpuInfo::print_to_klog() const {
    KernelLog& klog = KernelLog::instance();
    klog.format("CPU: % @ %MHz\n", get_vendor(), get_peak_mhz());
}

/**
 * @ref http://wiki.osdev.org/Detecting_CPU_Speed#Without_Interrupts
 */
u32 CpuInfo::get_peak_mhz() const {
    const u32 COUNTER_MAX = 0x10000;

    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();
    auto pit = driver_manager.get_driver<PitDriver>();

    if (!pit) {
        klog.format("CpuInfo::get_peak_mhz: no PitDriver\n");
        return 0;
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

    return cpu_hz  / 1000 / 1000;
}

string CpuInfo::get_vendor() const {
    string result {"____________"};

    __asm__ (
        "movq %0, %%rdi\n\t"
        "xor %%rax, %%rax;\n\t"
         "cpuid;\n\t"
         "mov %%ebx, (%%rdi);\n\t"
         "mov %%edx, 4(%%rdi);\n\t"
         "mov %%ecx, 8(%%rdi);\n\t"
        : // no output used
        : "g" (result.c_str())
        : "memory", "%eax", "%ebx", "%ecx", "%edx", "%rdi"
    );

    return result;
}

u64 CpuInfo::rtdsc() const {
    u32 hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)lo | ((u64)hi << 32));
}

} // namespace utils
