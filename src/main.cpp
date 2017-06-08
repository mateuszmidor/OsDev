/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"


/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    ScreenPrinter p;
    p.set_bg_color(Color::Blue);
    p.format("\n\n"); // go to the third line of console as 1 and 2 are used upon initializing in 32 and then 64 bit mode
    p.format("Hello in kmain() of main.cpp!\n");

    CpuInfo cpu_info;
    cpu_info.print(p);

    Multiboot2 mb2(multiboot2_info_ptr);
    mb2.print(p);
}
