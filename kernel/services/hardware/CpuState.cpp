/**
 *   @file: CpuState.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"
#include "CpuState.h"

namespace hardware {

/**
 * Constructor.
 * @param rip           Instruction pointer to be set by iretq in interrupts.S
 * @param rsp           Stack pointer to be set by iretq
 * @param task_arg      Value for RDI register (function first argument in SYSTEM V ABI)
 * @param user_space    Should the iretq jump to user space(true) or to kernel space(false), this effectively defines cs (code segment) and ss (stack segment)
 */
CpuState::CpuState(u64 rip, u64 rsp, u64 task_arg1, u64 task_arg2, bool user_space, u64 pml4_phys_addr) {
    rax = 0;
    rbx = 0;
    rcx = 0;
    rdx = 0;

    rdi = task_arg1;        // first argument passed to the task function in RDI according to SYSTEM V x64 ABI
    rsi = task_arg2;        // second argument passed to the task function in RDI according to SYSTEM V x64 ABI
    rbp = 0;

    r8 = 0;
    r9 = 0;
    r10 = 0;
    r11 = 0;
    r12 = 0;
    r13 = 0;
    r14 = 0;
    r15 = 0;
    cr3 = pml4_phys_addr == 0 ? 0xBAD00000BAD00000 : pml4_phys_addr;
    error_code  = 0;                                        // this is returned from CPU exceptions that come with error_code, otherwise just 0
    this->rip   = rip;                                      // this is return address for iretq instruction in interrupt handler in interrupts.S
    rflags      = 0x202;                                    // 1000000010; INTERRUPTS | BIT_1 (always set)
    this->rsp   = rsp;                                      // target stack pointer
    if (user_space) {
        cs = Gdt::get_user_code_segment_selector();         // target code segment selector in GDT
        ss = Gdt::get_user_data_segment_selector();
    }
    else {
        cs = Gdt::get_kernel_code_segment_selector();       // target code segment selector in GDT
        ss = Gdt::get_kernel_data_segment_selector();
    }
}
} /* namespace cpu */
