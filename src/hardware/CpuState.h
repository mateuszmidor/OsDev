/**
 *   @file: CpuState.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPU_CPUSTATE_H_
#define SRC_CPU_CPUSTATE_H_

#include "types.h"

namespace hardware {

/**
 * On CPU exception/PIC interrupt, following structure is layed on top of the stack before InterruptManager::on_interrupt is called
 * See: https://os.phil-opp.com/handling-exceptions/
 */
struct CpuState {
    CpuState(u64 rip = 0, u64 rsp = 0, u64 task_arg1 = 0, u64 task_arg2 = 0, bool user_space = false, u64 pml4_phys_addr = 0);

    // this first part we save/restore manually
    u64 cr3;
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;

    u64 rsi;
    u64 rdi;
    u64 rbp;

    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;


    // this second part is filled by CPU as part of exception/interrupt protocol
    // see https://os.phil-opp.com/handling-exceptions/
    u64 error_code; // only provided by CPU exceptions that carry error code
    u64 rip;        // this is iret return address that jump from interrupt to task to be executed
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((packed));

} /* namespace cpu */

#endif /* SRC_CPU_CPUSTATE_H_ */
