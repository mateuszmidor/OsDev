/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"


using namespace kstd;


// screen printer for printing to the screen
ScreenPrinter printer;


/**
 * Test kstd namespace functionality
 */
void test_kstd(ScreenPrinter &p) {
    // test vector
    vector<long long> vec;
    for (int i = 0; i < 10; i++)
        vec.push_back(i);
    for (auto a : vec)
        p.format("%, ", a);

    p.format("\n");

    // test string
    string s("abcdefghijklkmnoprstuwxyz");
    s += "123456abcdefghijklkmnoprstuwxyz";
    p.format("%\n", s.c_str());

    // test flags
    string s2= flags_to_str(5, "READ=0x4", "WRITE=0x2", "EXEC=0x1");
    p.format("%\n", s2.c_str());
}

/**
 * Test Interrupt Descriptor Table
 */
void test_idt() {
    // check breakpoint exception handler
    asm("int3");

    // check zero division exception handler
    asm("mov $0, %rcx; mov $0, %rdx; idiv %rcx");

    // check page fault exception handler (write to non mapped page)
    //asm("mov %rax, (1024*1024*1024)");
}

/**
 * @name    callGlobalConstructors
 * @brief   Call the constructors of objects defined in global namespace (if any)
 * @note    start_ctors and end_ctors defined in linker.ld
 */
typedef void (*Constructor)();
extern "C" Constructor start_ctors;
extern "C" Constructor end_ctors;
void callGlobalConstructors() {
    for (Constructor* c = &start_ctors; c != &end_ctors; c++)
        (*c)();
}

/**
 * @name    on_interrupt
 * @brief   Interrupt/CPU exception handler. This is called from interrupts.S
 */
extern "C" void on_interrupt(u8 interrupt_no, u64 error_code) {
    static u32 count = 1;
    printer.format("interrupt no. %, error code % [%]\n", interrupt_no, error_code, count++);
}

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // run constructors of global objects. This could be run from long_mode_init.S
    callGlobalConstructors();

    // configure Interrupt Descriptor Table
    configIDT();

    // print hello message to the user
    printer.set_bg_color(Color::Blue);
    printer.format("\n\n"); // go to the third line of console as 1 and 2 are used upon initializing in 32 and then 64 bit mode
    printer.format("Hello in kmain() of main.cpp!\n");

    // print CPU info
    CpuInfo cpu_info;
    cpu_info.print(printer);

    // print Multiboot2 info related to framebuffer config, available memory and kernel ELF sections
    Multiboot2 mb2(multiboot2_info_ptr);
    mb2.print(printer);

    //test_kstd(p);
    test_idt();

    // inform setup done
    printer.format("KERNEL SETUP DONE.\n");
}
