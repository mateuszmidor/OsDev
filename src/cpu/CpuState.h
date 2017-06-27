/**
 *   @file: CpuState.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPU_CPUSTATE_H_
#define SRC_CPU_CPUSTATE_H_

#include "types.h"

namespace cpu {

/**
 * On CPU exception/PIC interrupt, following structure is layed on top of the stack
 */
struct CpuState {
    CpuState(u64 rip);

    // this first part we save/restore manually
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

    // this second part is part of exception/interrupt protocol; see https://os.phil-opp.com/handling-exceptions/
    u64 error_code;  // only provided by CPU exceptions that carry error code
    u64 rip;    // this is iret return address while finishing handling the interrupt
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((packed));

} /* namespace cpu */

#endif /* SRC_CPU_CPUSTATE_H_ */
