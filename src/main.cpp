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
    asm("xor %rbx, %rbx; idiv %rbx");
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

IdtEntry idt[16] ;

struct IdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed)) ;

//extern "C" void* divide_by_zero_handler(void *rsp) {
//    ScreenPrinter p;
//    p.format("ZERO DIVISION\n");
//
//    return rsp;
//}

extern "C" void handle_interrupt();

void configIDT() {
    idt[0] = make_entry((u64)handle_interrupt); // zero division
    idt[3] = make_entry((u64)handle_interrupt); // breakpoint

    IdtSizeAddress idt_size_address ;
    idt_size_address.size_minus_1 = sizeof(idt) - 1;
    idt_size_address.address = (u64)idt;

    __asm__ volatile("lidt %0"
            :
            : "m" (idt_size_address)
            );
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
    test_idt();

    // print hello message to the user
    ScreenPrinter p;
    p.set_bg_color(Color::Blue);
    p.format("\n\n"); // go to the third line of console as 1 and 2 are used upon initializing in 32 and then 64 bit mode
    p.format("Hello in kmain() of main.cpp!\n");

    // print CPU info
    CpuInfo cpu_info;
    cpu_info.print(p);

    // print Multiboot2 info related to framebuffer config, available memory and kernel ELF sections
    Multiboot2 mb2(multiboot2_info_ptr);
    mb2.print(p);

    //test_kstd(p);

}
