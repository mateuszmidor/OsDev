/**
 *   @file: CpuInfo.cpp
 *
 *   @date: Jun 7, 2017
 * @author: mateusz
 */

#include "KernelLog.h"
#include "CpuInfo.h"

namespace utils {

CpuInfo::CpuInfo() {
    getCpuVendor(vendor);
}

void CpuInfo::print_to_klog() const {
    KernelLog& klog = KernelLog::instance();
    klog.format("CPU: %\n", vendor);
}

void CpuInfo::getCpuVendor(char buff[13]) {
    buff[12] = '\0';

    __asm__ (
        "movq %0, %%rdi\n\t"
        "xor %%rax, %%rax;\n\t"
         "cpuid;\n\t"
         "mov %%ebx, (%%rdi);\n\t"
         "mov %%edx, 4(%%rdi);\n\t"
         "mov %%ecx, 8(%%rdi);\n\t"
        : // no output used
        : "g" (vendor)
        : "memory", "%eax", "%ebx", "%ecx", "%edx", "%rdi"
    );
}

} // namespace utils
