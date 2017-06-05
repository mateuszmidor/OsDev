/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"

void getCpuVendor(char buff[17]) {
    buff[16] = '\0';

    __asm__ (
        "movq %0, %%rdi\n\t"
        "xor %%rax, %%rax;\n\t"
         "cpuid;\n\t"
         "mov %%ebx, (%%rdi);\n\t"
         "mov %%edx, 4(%%rdi);\n\t"
         "mov %%ecx, 8(%%rdi);\n\t"
        : // no output used
        : "g" (buff)
        : "memory", "%eax", "%ebx", "%ecx", "%edx", "%rdi"
    );
}
/**
 * main
 * Kernel entry point
 */
int main() {
    ScreenPrinter p;
    p.format("\nHello in main() of main.cpp!\n");

    char cpu_vendor_cstr[17];
    getCpuVendor(cpu_vendor_cstr);
    p.set_bg_color(Color::Blue);
    p.format("CPU: %\n", cpu_vendor_cstr);
}

