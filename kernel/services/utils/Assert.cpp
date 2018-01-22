/**
 *   @file: Assert.cpp
 *
 *   @date: Nov 16, 2017
 * @author: Mateusz Midor
 */

#include "Assert.h"

namespace utils {

void phobos_assert(bool condition, const char* msg) {
    if (condition)
        return;

    const u16 RED_ON_WHITE = (15 << 12) | (4 << 8);

    u16* vga = (u16*)0xFFFFFFFF800b8000;
    while (*msg) {
        *vga =  *msg | RED_ON_WHITE;
        vga++;
        msg++;
    }

    const char* system_halt = " [System Halted] ";
    while (*system_halt) {
        *vga =  *system_halt | RED_ON_WHITE;
        vga++;
        system_halt++;
    }

    // halt the kernel
    phobos_halt();
}

void phobos_halt() {
    asm volatile ("cli; hlt");
}
} /* namespace utils */
