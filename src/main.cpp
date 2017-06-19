/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
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
 * Interrupt Descriptor Table Entry Options
 */
struct IdtEntryOptions {
    u16 interrupt_stack_table_index : 3;    // 0 for DONT CHANGE THE STACKS
    u16 reserved                    : 5;    // always 0
    u16 interrupts_enabled          : 1;    // better be 0 for now
    u16 always_1                    : 3;    // always 0b111
    u16 always_0                    : 1;    // always 0
    u16 min_privilege_level         : 2;    // 3 for user space
    u16 present                     : 1;    // 1 for true

    IdtEntryOptions(bool is_present = false) {
        interrupt_stack_table_index = 0;
        reserved = 0;
        interrupts_enabled = 0;
        always_1 = 7; // 0b111
        always_0 = 0;
        min_privilege_level = 0;
        present = is_present;

    }
} __attribute__((packed));

/**
 * Interrupt Descriptor Table Entry
 */
struct IdtEntry {
    u16 pointer_low;
    u16 gdt_code_segment_selector;  // offset of code segment in GDT, in our case 8
    IdtEntryOptions options;
    u16 pointer_middle;
    u32 pointer_high;
    u32 always_0;

} __attribute__((packed));

IdtEntry make_entry(u64 pointer) {
    IdtEntry e;

    e.gdt_code_segment_selector = 8; // at offset 8 starts our one and the only code segment entry for 64 bit long mode
    e.pointer_low = pointer & 0xFFFF;
    e.pointer_middle = (pointer >> 16) & 0xFFFF;
    e.pointer_high = (pointer >> 32) & 0xFFFFFFFF;
    e.options = IdtEntryOptions(true);
    e.always_0 = 0;

    return e;
}

IdtEntry idt[0x14] ;

struct IdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed)) ;

extern "C" void handle_exception_no_0x00();
extern "C" void handle_exception_no_0x01();
extern "C" void handle_exception_no_0x02();
extern "C" void handle_exception_no_0x03();
extern "C" void handle_exception_no_0x04();
extern "C" void handle_exception_no_0x05();
extern "C" void handle_exception_no_0x06();
extern "C" void handle_exception_no_0x07();
extern "C" void handle_exception_no_0x08();
extern "C" void handle_exception_no_0x09();
extern "C" void handle_exception_no_0x0A();
extern "C" void handle_exception_no_0x0B();
extern "C" void handle_exception_no_0x0C();
extern "C" void handle_exception_no_0x0D();
extern "C" void handle_exception_no_0x0E();
extern "C" void handle_exception_no_0x0F();
extern "C" void handle_exception_no_0x10();
extern "C" void handle_exception_no_0x11();
extern "C" void handle_exception_no_0x12();
extern "C" void handle_exception_no_0x13();

void configIDT() {
    idt[0x00] = make_entry((u64)handle_exception_no_0x00); // zero division
    idt[0x01] = make_entry((u64)handle_exception_no_0x01);
    idt[0x02] = make_entry((u64)handle_exception_no_0x02);
    idt[0x03] = make_entry((u64)handle_exception_no_0x03); // breakpoint
    idt[0x04] = make_entry((u64)handle_exception_no_0x04);
    idt[0x05] = make_entry((u64)handle_exception_no_0x05);
    idt[0x06] = make_entry((u64)handle_exception_no_0x06);
    idt[0x07] = make_entry((u64)handle_exception_no_0x07);
    idt[0x08] = make_entry((u64)handle_exception_no_0x08);
    idt[0x09] = make_entry((u64)handle_exception_no_0x09);
    idt[0x0A] = make_entry((u64)handle_exception_no_0x0A);
    idt[0x0B] = make_entry((u64)handle_exception_no_0x0B);
    idt[0x0C] = make_entry((u64)handle_exception_no_0x0C);
    idt[0x0D] = make_entry((u64)handle_exception_no_0x0D);
    idt[0x0E] = make_entry((u64)handle_exception_no_0x0E);
    idt[0x0F] = make_entry((u64)handle_exception_no_0x0F);
    idt[0x10] = make_entry((u64)handle_exception_no_0x10);
    idt[0x11] = make_entry((u64)handle_exception_no_0x11);
    idt[0x12] = make_entry((u64)handle_exception_no_0x12);
    idt[0x13] = make_entry((u64)handle_exception_no_0x13);

    IdtSizeAddress idt_size_address ;
    idt_size_address.size_minus_1 = sizeof(idt) - 1;
    idt_size_address.address = (u64)idt;

    __asm__ volatile("lidt %0"
            :
            : "m" (idt_size_address)
            );
}

extern "C" void on_interrupt(u8 int_no) {
    static u32 count = 1;
    printer.format("interrupt no. % [%]\n", int_no, count++);
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
