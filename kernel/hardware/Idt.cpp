/**
 *   @file: Idt.cpp
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"
#include "Idt.h"

// CPU exceptions defined in "interrupts.S"
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
extern "C" void handle_exception_no_0x14();
extern "C" void handle_exception_no_0x1E();

// PIC interrupts, shifted by IRQ_BASE, defined in "interrupts.S"
extern "C" void ignore_interrupt();
extern "C" void handle_interrupt_no_0x20();
extern "C" void handle_interrupt_no_0x21();
extern "C" void handle_interrupt_no_0x2C();
extern "C" void handle_interrupt_no_0x2E();
extern "C" void handle_interrupt_no_0x2F();
extern "C" void handle_interrupt_no_0x80(); // old fashioned syscall "int 0x80", now we use "syscall"

namespace hardware {

void Idt::reinstall_idt() {
    setup_interrupt_descriptor_table();
    install_interrupt_descriptor_table();
}

/**
 * @brief   Make Interrupt Descriptor Table gate
 * @param   handler_pointer Interrupt handler routine (naked function in asm)
 * @param   ist_index Interrupt Stack Table index [1..7] for emergency situations handling, or 0 if no IST is used (default)
 * @param   min_privilege_level Minimum privilege level required to raise this interrupt (eg 3 is needed for user space tasks to raise int 0x80)
 * @return  IDT gate
 */
IdtEntry Idt::make_entry(u64 handler_pointer, u8 ist_index, u8 min_privilege_level) {
    IdtEntry e;

    e.gdt_code_segment_selector = Gdt::get_kernel_code_segment_selector();
    e.pointer_low = handler_pointer & 0xFFFF;
    e.pointer_middle = (handler_pointer >> 16) & 0xFFFF;
    e.pointer_high = (handler_pointer >> 32) & 0xFFFFFFFF;
    e.options = IdtEntryOptions(min_privilege_level, true, ist_index);
    e.reserved = 0;

    return e;
}

void Idt::setup_interrupt_descriptor_table() {
    // CPU exceptions
    idt[0x00] = make_entry((u64) (handle_exception_no_0x00));       // Divide-by-zero Error
    idt[0x01] = make_entry((u64) (handle_exception_no_0x01));       // Debug
    idt[0x02] = make_entry((u64) (handle_exception_no_0x02), 1);    // Non-maskable Interrupt; use kernel emergency stack 1 (makes "syscall" handling safer)
    idt[0x03] = make_entry((u64) (handle_exception_no_0x03));       // Breakpoint
    idt[0x04] = make_entry((u64) (handle_exception_no_0x04));       // Overflow
    idt[0x05] = make_entry((u64) (handle_exception_no_0x05));       // Bound Range Exceeded
    idt[0x06] = make_entry((u64) (handle_exception_no_0x06));       // Invalid Opcode
    idt[0x07] = make_entry((u64) (handle_exception_no_0x07));       // Device Not Available
    idt[0x08] = make_entry((u64) (handle_exception_no_0x08), 1);    // Double Fault; use kernel emergency stack 1 (prevent triple fault)
    idt[0x09] = make_entry((u64) (handle_exception_no_0x09));       // Coprocessor Segment Overrun
    idt[0x0A] = make_entry((u64) (handle_exception_no_0x0A));       // Invalid TSS
    idt[0x0B] = make_entry((u64) (handle_exception_no_0x0B));       // Segment Not Present
    idt[0x0C] = make_entry((u64) (handle_exception_no_0x0C));       // Stack-Segment Fault
    idt[0x0D] = make_entry((u64) (handle_exception_no_0x0D), 1);    // General Protection Fault; use kernel emergency stack 1 (makes "syscall" handling safer)
    idt[0x0E] = make_entry((u64) (handle_exception_no_0x0E));       // Page Fault
    idt[0x0F] = make_entry((u64) (handle_exception_no_0x0F));       // Reserved
    idt[0x10] = make_entry((u64) (handle_exception_no_0x10));       // x87 Floating-Point Exception
    idt[0x11] = make_entry((u64) (handle_exception_no_0x11));       // Alignment Check
    idt[0x12] = make_entry((u64) (handle_exception_no_0x12), 1);    // Machine Check; use kernel emergency stack 1 (makes "syscall" handling safer)
    idt[0x13] = make_entry((u64) (handle_exception_no_0x13));       // SIMD Floating-Point Exception
    idt[0x14] = make_entry((u64) (handle_exception_no_0x14));       // Virtualization Exception
    idt[0x1E] = make_entry((u64) (handle_exception_no_0x1E));       // Security Exception

    // PIC interrupts, they start at IRQ_BASE = 0x20, defined in interrupts.S
    // first setup idt to ignore all interrupts
    for (u32 i = Interrupts::IRQ_BASE; i < Interrupts::IRQ_MAX; i++)
        idt[i] = make_entry((u64) (ignore_interrupt));

    // then handle just the interrupts of interest
    idt[Interrupts::PIT]            = make_entry((u64) (handle_interrupt_no_0x20));         // timer
    idt[Interrupts::Keyboard]       = make_entry((u64) (handle_interrupt_no_0x21));         // keyboard
    idt[Interrupts::Mouse]          = make_entry((u64) (handle_interrupt_no_0x2C));         // mouse
    idt[Interrupts::PrimaryAta]     = make_entry((u64) (handle_interrupt_no_0x2E));         // primary ata bus
    idt[Interrupts::SecondaryAta]   = make_entry((u64) (handle_interrupt_no_0x2F));         // secondary ata bus
    idt[Interrupts::Int80h]        = make_entry((u64) (handle_interrupt_no_0x80), 0, 3);   // "int 0x80"; regular kernel stack, min privilege=user space
}

void Idt::install_interrupt_descriptor_table() {
    IdtSizeAddress idt_size_address;
    idt_size_address.size_minus_1 = sizeof(IdtEntry) * idt.size() - 1;
    idt_size_address.address = (u64) (idt.data());

    asm("lidt %0" : : "m" (idt_size_address));
}
} /* namespace hardware */
