/**
 *   @file: Sse.cpp
 *
 *   @date: Sep 15, 2017
 * @author: Mateusz Midor
 */

#include "CpuInfo.h"
#include "Sse.h"

namespace utils {

/**
 * @brief   Activate the legacy SSE
 * @return  True if legacy sse is supported by CPU and activated, False otherwise
 * @see     http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, 11.3.1  Enabling Legacy SSE Instruction Execution
 */
bool Sse::activate_legacy_sse() {
    // first check for sse support
    CpuInfo cpu_info;
    if (!cpu_info.get_multimedia_extensions().sse)
        return false;

    asm volatile (
            // enable FX SAVE/FXRSTOR (CR4 bit 9)
            "mov %%cr4, %%rax                     ;"
            "or $0x200, %%rax                     ;"
            "mov %%rax, %%cr4                     ;"

            // disable emulate coprocessor (CR0 bit 2)
            "mov %%cr0, %%rax                     ;"
            "and $0xFFFFFFFFFFFFFFFB, %%rax       ;"
            "mov %%rax, %%cr0                     ;"

            // enable monitor coprocessor (CR0 bit 1)
            "mov %%cr0, %%rax                     ;"
            "or $0x2, %%rax                       ;"
            "mov %%rax, %%cr0                     ;"
            :
            :
            : "%rax"
    );

    return true;
}
} /* namespace utils */
