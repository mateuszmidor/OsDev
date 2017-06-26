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
#include "ExceptionManager.h"
#include "VgaDriver.h"

using namespace kstd;


// screen printer for printing to the screen
ScreenPrinter &printer = ScreenPrinter::instance();

cpuexceptions::ExceptionManager exception_manager = cpuexceptions::ExceptionManager::instance();
InterruptManager &interrupt_manager = InterruptManager::instance();
drivers::DriverManager &driver_manager = drivers::DriverManager::instance();
drivers::KeyboardScanCodeSet1 scs1;
auto keyboard = std::make_shared<drivers::KeyboardDriver> (scs1);
auto mouse = std::make_shared<drivers::MouseDriver>();
drivers::VgaDriver vga;

s16 mouse_x = 360;
s16 mouse_y = 200;


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

void on_mouse_move_text(s8 dx, s8 dy) {
    const u8 CHAR_WIDTH = 720 / vga.screen_width();
    const u8 CHAR_HEIGHT = 400 / vga.screen_height();
    printer.swap_fg_bg_at(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
    mouse_x += dx ;
    mouse_y += dy;
    if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0; if (mouse_x > 719) mouse_x = 719; if (mouse_y > 399) mouse_y = 399;
    printer.swap_fg_bg_at(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
}

drivers::EgaColor pen_color = drivers::EgaColor::LightRed;
void on_mouse_move_graphics(s8 dx, s8 dy) {
    mouse_x += dx ;
    mouse_y += dy;
    if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > vga.screen_width()) mouse_x = vga.screen_width(); if (mouse_y > vga.screen_height()) mouse_y = vga.screen_height();

    vga.put_pixel(mouse_x, mouse_y, pen_color);
}

void on_mouse_down_text(u8 button) {
    const u8 CHAR_WIDTH = 720 / vga.screen_width();
    const u8 CHAR_HEIGHT = 400 / vga.screen_height();
    printer.move_to(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
}

void on_mouse_down_graphics(u8 button) {
    switch (button) {
    default:
    case drivers::MouseButton::LEFT:    pen_color = drivers::EgaColor::LightRed; break;
    case drivers::MouseButton::RIGHT:   pen_color = drivers::EgaColor::LightGreen; break;
    case drivers::MouseButton::MIDDLE:  pen_color = drivers::EgaColor::LightBlue; break;
    }
}

void on_key_press(s8 key) {
    char s[2] = {key, '\0'};
    printer.format("%", s);
}

void vga_demo() {
    // vga demo
    mouse->set_on_move(on_mouse_move_graphics);
    mouse->set_on_down(on_mouse_down_graphics);
    vga.set_graphics_mode_320_200_256();
    mouse_x = 320 / 2;
    mouse_y = 200 / 2;
    for (u16 x = 0; x < vga.screen_width(); x++)
        for (u16 y = 0; y < vga.screen_height(); y++)
            vga.put_pixel(x, y, (x > 315 || x < 4 || y > 195 || y < 4) ? drivers::EgaColor::LightGreen : drivers::EgaColor::Black); // 4 pixels thick frame around the screen
}

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // run constructors of global objects. This could be run from long_mode_init.S
    GlobalConstructorsRunner::run();

    // configure drivers
    keyboard->set_on_key_press(on_key_press);
    mouse->set_on_move(on_mouse_move_text);
    mouse->set_on_down(on_mouse_down_text);

    // install drivers
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);

    // configure Interrupt Descriptor Table
    interrupt_manager.set_exception_handler([] (u8 exc_no, u64 error) { exception_manager.on_exception(exc_no, error); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no) { driver_manager.on_interrupt(int_no); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts();

    // print hello message to the user
    vga.set_text_mode_90_30();
    printer.clearscreen();
    printer.set_bg_color(drivers::EgaColor::Blue);

    // print CPU info
    CpuInfo cpu_info;
    cpu_info.print(printer);

    // print Multiboot2 info related to framebuffer config, available memory and kernel ELF sections
    Multiboot2 mb2(multiboot2_info_ptr);
    mb2.print(printer);

    // print PCI devics
    PCIController pcic;
    pcic.select_drivers();

    //test_kstd(p);
    // test interrupt descriptor table
    test_idt();

    // inform setup done
    printer.format("KERNEL SETUP DONE.\n");

    // vga demo
    vga_demo();

    // loop forever handling interrupts
    for(;;)
       asm("hlt");
}
