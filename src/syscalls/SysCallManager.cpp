/**
 *   @file: SysCallManager.cpp
 *
 *   @date: Sep 13, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"
#include "KernelLog.h"
#include "SysCallManager.h"
#include "Task.h"

using namespace utils;
using namespace multitasking;
namespace syscalls {

/**
 * @brief   Raw syscall handler that:
 *          1. saves user task context
 *          2. switches to kernel stack
 *          3. calls on_syscall
 *          4. switches back to user stack
 *          5. restores user task context
 *          implemented in syscalls.S
 */
extern "C" void handle_syscall();

/**
 * @brief   "syscall" handler. This is called from syscalls.S
 * @note    This is run in kernel space, using kernel stack
 */
extern "C" s64 on_syscall(u64 sys_call_num)  {
    SysCallManager& mngr = SysCallManager::instance();
    KernelLog& klog = KernelLog::instance();

    klog.format("syscall_handler: % \n", sys_call_num);

    if (sys_call_num == 60) {
//        Task::exit();   // not sure if interrupt from syscall is the right way to go...
    }

    return 0;
}


SysCallManager SysCallManager::_instance;

SysCallManager& SysCallManager::instance() {
    return _instance;
}

/**
 * @brief   Configure syscall-related Model Specific Registers and enable the "syscall/sysret" instructions in CPU
 */
void SysCallManager::config_and_activate_syscalls() {
    hardware::Gdt gdt;
    MSR_STAR s_star;
    s_star.syscall_cs_ss = gdt.get_kernel_code_segment_selector() ;
    s_star.sysret_cs_ss = gdt.get_user_data_segment_selector() - 8;
    s_star.syscall_target_eip_32bit = 0;

    u32 mask = 0x200; // disable interrupts
    u64 lstar = (u64)handle_syscall;
    u32 lstar_lo = lstar & 0xFFFFFFFF;
    u32 lstar_hi = lstar >> 32;
    u64 star;
    memcpy(&star, &s_star, sizeof(s_star));
    u32 star_lo = star & 0xFFFFFFFF;
    u32 star_hi = star >> 32;

    asm volatile (
            // write SFMASK to MSR 0xC000_0084
            "mov $0xC0000084, %%ecx     \n;"
            "mov %0, %%rax              \n"
            "wrmsr                      \n"

            // write CSTAR to MSR 0xC000_0083 - only for 32bit compatibility mode; not used in our system :)
            // "mov $0xC0000083, %%ecx     \n;"
            // "mov $0, %%eax              \n"
            // "mov %0, %%edx              \n"
            // "wrmsr                      \n"

            // write LSTAR to MSR 0xC000_0082
            "mov $0xC0000082, %%ecx     \n;"
            "mov %1, %%eax              \n"     // lstar_lo
            "mov %2, %%edx              \n"     // lstar_hi
            "wrmsr                      \n"

            // write STAR to MSR 0xC000_0081
            "mov $0xC0000081, %%ecx     \n;"
            "mov %3, %%eax              \n"     // star_lo
            "mov %4, %%edx              \n"     // star_hi
            "wrmsr                      \n"

            // enable syscall instruction in EFER Model Specific Register, otherwise Invalid Opcode exception 0x6 would be raised on syscall
            "mov $0xC0000080, %%ecx     \n;"
            "rdmsr                      \n;"
            "or $1, %%eax               \n"
            "wrmsr                      \n"
            :
            : "m"(mask), "m"(lstar_lo), "m"(lstar_hi), "m"(star_lo), "m"(star_hi)
            : "memory", "%rax", "%rcx", "%rdx"
    );
}

} /* namespace syscalls */
