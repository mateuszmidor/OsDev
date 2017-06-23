/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "GlobalConstructorsRunner.h"
#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "PCIController.h"

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

s16 mouse_x = 360;
s16 mouse_y = 200;
void on_mouse_move(s8 dx, s8 dy) {
    printer.swap_fg_bg_at(mouse_x / 9, mouse_y / 16); // 9x16 is character size
    mouse_x += dx ;
    mouse_y += dy;
    if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0; if (mouse_x > 719) mouse_x = 719; if (mouse_y > 399) mouse_y = 399;
    printer.swap_fg_bg_at(mouse_x / 9, mouse_y / 16);
}

void on_mouse_down(u8 button) {
    printer.move_to(mouse_x / 9, mouse_y / 16);
}

void on_key_press(s8 key) {
    char s[2] = {key, '\0'};
    printer.format("%", s);
}

InterruptManager &interrupt_manager = InterruptManager::instance();
drivers::DriverManager &driver_manager = drivers::DriverManager::instance();
drivers::KeyboardScanCodeSet1 scs1;
auto keyboard = std::make_shared<drivers::KeyboardDriver> (scs1);
auto mouse = std::make_shared<drivers::MouseDriver>();

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // run constructors of global objects. This could be run from long_mode_init.S
    GlobalConstructorsRunner::run();

    // configure drivers
    keyboard->set_on_key_press(on_key_press);
    mouse->set_on_move(on_mouse_move);
    mouse->set_on_down(on_mouse_down);

    // install drivers
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);

    // configure Interrupt Descriptor Table
    interrupt_manager.set_interrupt_handler([] (u8 int_no) { driver_manager.on_interrupt(int_no); } );
    interrupt_manager.config_and_activate_interrupts();


    // print hello message to the user
    printer.set_bg_color(Color::Blue);
    printer.format("\n\n"); // go to the third line of console as 1 and 2 are used upon initializing in 32 and then 64 bit mode
    printer.format("Hello in kmain() of main.cpp!\n");

    // print CPU info
    CpuInfo cpu_info;
    //cpu_info.print(printer);

    // print Multiboot2 info related to framebuffer config, available memory and kernel ELF sections
    Multiboot2 mb2(multiboot2_info_ptr);
    //mb2.print(printer);


    PCIController pcic;
    pcic.select_drivers();
    //test_kstd(p);
    //test_idt();

    // inform setup done
    printer.format("KERNEL SETUP DONE.\n");

    // loop forever handling interrupts
    for(;;)
       asm("hlt");
}
