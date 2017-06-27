/**
 *   @file: CpuState.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "CpuState.h"

namespace cpu {

CpuState::CpuState (u64 rip = 0) {
    rax = 0;
    rbx = 0;
    rcx = 0;
    rdx = 0;

    rsi = 0;
    rdi = 0;
    rbp = 0;

    r8 = 0;
    r9 = 0;
    r10 = 0;
    r11 = 0;

    error_code = 0;     // this is returned from CPU exceptions that come with error_code
    this->rip = rip;    // this is return address for iret instruction
    cs = 8;             // for long mode we have only null segment(gdt offset 0) and code segment(gdt offset 8)
    rflags = 0x202;     // 1000000010; INTERRUPTS | BIT_1 (always set)
    rsp = 0;
    ss = 0;
}
} /* namespace cpu */
