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
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"
#include "ScanCodeSet1.h"

using namespace kstd;


// screen printer for printing to the screen
ScreenPrinter printer;
ScanCodeSet1 scan_code_set1;

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


void setup_mouse() {
    Port8bit mouse_cmd_port(0x64);
    Port8bit mouse_data_port(0x60);

    mouse_cmd_port.write(0xA8);
    mouse_cmd_port.write(0x20);
    u8 status = mouse_data_port.read() | 0x02;
    mouse_cmd_port.write(0x60);
    mouse_data_port.write(status);

    mouse_cmd_port.write(0xD4);
    mouse_data_port.write(0xF4);
    if (mouse_data_port.read() != 0xFA)
        printer.format("mouse init error\n");
}
/**
 * Minimal interrupt handling routine
 */
s16 x = 0, y = 0;
u8 buttons = 0;
u8 offset = 0;
u8 buffer[3];
Port8bit mouse_cmd_port(0x64);
Port8bit mouse_data_port(0x60);

void on_interrupt(u8 interrupt_no, u64 error_code) {
    static u32 count = 1;


    switch (interrupt_no) {
    case 32: {  // timer
        return;
    }

    case 33: {  // keyboard
        Port8bit keyboard_key_port(0x60);
        u8 key_code = keyboard_key_port.read();
        printer.format("%", scan_code_set1.code_to_ascii(key_code).c_str());
        break;
    }

    case 44: {  // mouse

        u8 status = mouse_cmd_port.read();
        if (!(status & 0x01) || !(status & 0x20)) // check for mice data available
            return;

        buffer[offset] = mouse_data_port.read();
        offset = (offset + 1) % 3;
        if (offset == 0) {
            x += (buffer[1] - ((buffer[0] << 4) & 0x100)) ;
            y -= (buffer[2] - ((buffer[0] << 3) & 0x100)) ;
            printer.format("[%, %]                      \n", x, y);

            for (u8 i = 0; i < 3; i++) {

                if ((buffer[0] & (1 << i)) && !(buttons & (1 << i)))
                    printer.format("button % down\n", i);

                if (!(buffer[0] & (1 << i)) && (buttons & (1 << i)))
                    printer.format("button % up\n", i);
            }
            buttons = buffer[0];
        }

       // printer.format("mouse ");
        break;
    }

    default: {
        printer.format("interrupt no. %, error code % [%]\n", interrupt_no, error_code, count++);
    }
    }


}

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // run constructors of global objects. This could be run from long_mode_init.S
    GlobalConstructorsRunner::run();


    // configure Interrupt Descriptor Table
    InterruptManager::set_handler(on_interrupt);
    InterruptManager::config_interrupts();
    setup_mouse();
    InterruptManager::activate_interrupts();


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
    //test_idt();

    // inform setup done
    printer.format("KERNEL SETUP DONE.\n");

    // loop forever handling interrupts
    for(;;)
       asm("hlt");
}
