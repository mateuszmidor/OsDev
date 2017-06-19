/**
 *   @file: InterrupManager.cpp
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#include "InterruptManager.h"


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


IdtEntry idt[0x14] ;

IdtEntry make_entry(u64 pointer, u16 code_segment_selector = 8) {
    IdtEntry e;

    e.gdt_code_segment_selector = code_segment_selector; // at offset 8 starts our one and the only code segment entry for 64 bit long mode
    e.pointer_low = pointer & 0xFFFF;
    e.pointer_middle = (pointer >> 16) & 0xFFFF;
    e.pointer_high = (pointer >> 32) & 0xFFFFFFFF;
    e.options = IdtEntryOptions(true);
    e.always_0 = 0;

    return e;
}

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
