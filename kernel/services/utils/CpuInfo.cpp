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

using namespace cstd;
using namespace drivers;
using namespace logging;

namespace utils {

/**
 * @brief   String representation of supported CPU multimedia extensions
 */
string CpuMultimediaExtensions::to_string() const {
    string result;
    if (sse)    result += "SSE ";
    if (sse2)   result += "SSE2 ";
    if (sse3)   result += "SSE3 ";
    if (sse4_1) result += "SSE4.1 ";
    if (sse4_2) result += "SSE4.2 ";
    if (sse4a)  result += "SSE4a ";
    if (sse5)   result += "SSE5 ";
    if (avx)    result += "AVX ";
    return result;
}

void CpuInfo::print_to_klog() const {
    KernelLog& klog = KernelLog::instance();
    klog.format("CPU: % @ %MHz\n", get_vendor(), get_peak_mhz());
}

/**
 * @brief   Determine approximate peak CPU speed in MHz
 * @see     http://wiki.osdev.org/Detecting_CPU_Speed#Without_Interrupts
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

/**
 * @brief   Get CPU vendor string
 */
string CpuInfo::get_vendor() const {
    string result {"____________"};

    asm volatile (
         "movq %0, %%rdi;            "
         "xor %%rax, %%rax;          "
         "cpuid;                     "
         "mov %%ebx, (%%rdi);        "
         "mov %%edx, 4(%%rdi);       "
         "mov %%ecx, 8(%%rdi);       "
        : // no output used
        : "g" (result.c_str())
        : "memory", "%eax", "%ebx", "%ecx", "%edx", "%rdi"
    );

    return result;
}

/**
 * @brief   Get supported CPU multimedia extensions (SSE)
 * @note    All this multimedia detecting code is taken from:
 *          https://gist.github.com/hi2p-perim/7855506
 */
CpuMultimediaExtensions CpuInfo::get_multimedia_extensions() const {

    CpuMultimediaExtensions ext;
    int cpuinfo[4];
    __cpuid(cpuinfo, 1);

    // Check SSE, SSE2, SSE3, SSSE3, SSE4.1, and SSE4.2 support
    ext.sse       = cpuinfo[3] & (1 << 25);
    ext.sse2      = cpuinfo[3] & (1 << 26);
    ext.sse3      = cpuinfo[2] & (1 << 0);
    ext.ssse3     = cpuinfo[2] & (1 << 9);
    ext.sse4_1    = cpuinfo[2] & (1 << 19);
    ext.sse4_2    = cpuinfo[2] & (1 << 20);

    // ----------------------------------------------------------------------

    // Check AVX support
    // References
    // http://software.intel.com/en-us/blogs/2011/04/14/is-avx-enabled/
    // http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/

    ext.avx = cpuinfo[2] & (1 << 28);
    bool osxsaveSupported = cpuinfo[2] & (1 << 27);
    if (osxsaveSupported && ext.avx) {
        // _XCR_XFEATURE_ENABLED_MASK = 0
        unsigned long long xcrFeatureMask = _xgetbv(0);
        ext.avx = (xcrFeatureMask & 0x6) == 0x6;
    }

    // ----------------------------------------------------------------------

    // Check SSE4a and SSE5 support

    // Get the number of valid extended IDs
    __cpuid(cpuinfo, 0x80000000);
    int numExtendedIds = cpuinfo[0];
    if (numExtendedIds >= 0x80000001) {
        __cpuid(cpuinfo, 0x80000001);
        ext.sse4a = cpuinfo[2] & (1 << 6);
        ext.sse5 = cpuinfo[2] & (1 << 11);
    }

    // ----------------------------------------------------------------------

    return ext;
}

/**
 * @brief   Read Time Stamp Counter that is incremented by CPU on every cycle
 */
u64 CpuInfo::rtdsc() const {
    u32 hi, lo;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)lo | ((u64)hi << 32));
}

/**
 * @see     https://gist.github.com/hi2p-perim/7855506
 */
void CpuInfo::__cpuid(int* cpuinfo, int info) const {
    asm volatile(
        "xchg %%ebx, %%edi;"
        "cpuid;"
        "xchg %%ebx, %%edi;"
        :"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
        :"0" (info)
        :"%rbx" // this missing used to cause page fault at 2054MB in lscpu terminal command
    );
}

/**
 * @see     https://gist.github.com/hi2p-perim/7855506
 */
unsigned long long CpuInfo::_xgetbv(unsigned int index) const {
    unsigned int eax, edx;
    asm volatile(
        "xgetbv;"
        : "=a" (eax), "=d"(edx)
        : "c" (index)
    );
    return ((unsigned long long)edx << 32) | eax;
}

} // namespace utils
