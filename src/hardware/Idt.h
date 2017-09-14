/**
 *   @file: Idt.h
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_HARDWARE_IDT_H_
#define SRC_HARDWARE_IDT_H_

#include <array>
#include "InterruptNumbers.h"

namespace hardware {


/**
 * @brief   Interrupt Descriptor Table Entry Options
 * @see     https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf, Table 3-2. System-Segment and Gate-Descriptor Types
 */
struct IdtEntryOptions {
    u16 interrupt_stack_table_index : 3;    // 0 = use modified legacy stack-switching mechanism instead of interrupt stack table
    u16 reserved                    : 5;    // always 0
    u16 type                        : 4;    // 14 for 64-bit Interrupt Gate
    u16 always_0                    : 1;    // Storage Segment. Set to 0 for interrupt and trap gates.
    u16 min_privilege_level         : 2;    // 3 for user space
    u16 present                     : 1;    // 1 for true

    IdtEntryOptions(u8 privilege = 0, bool is_present = false, u8 ist_index = 0) {
        interrupt_stack_table_index = ist_index;
        reserved = 0;
        type = 14;
        always_0 = 0;
        min_privilege_level = privilege; // minimum privilege from which this interrupt can be emitted
        present = is_present;

    }
} __attribute__((packed));

/**
 * @brief   Interrupt Descriptor Table Entry
 * @see     http://wiki.osdev.org/Interrupt_Descriptor_Table
 *          https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf, Figure 5-7. 64-Bit IDT Gate Descriptors
 */
struct IdtEntry {
    u16 pointer_low;
    u16 gdt_code_segment_selector;  // offset of code segment in GDT, see Gdt class
    IdtEntryOptions options;
    u16 pointer_middle;
    u32 pointer_high;
    u32 reserved;   // set to 0

} __attribute__((packed));

/**
 * Struct to be loaded with lidt instruction
 */
struct IdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed)) ;


/**
 * @brief   Interrupt Descriptor Table abstraction
 */
class Idt {
public:
    void reinstall_idt();

private:
    IdtEntry make_entry(u64 handler_pointer, u8 ist_index = 0, u8 min_privilege_level = 0);
    void setup_interrupt_descriptor_table();
    void install_interrupt_descriptor_table();

    std::array<IdtEntry, Interrupts::IRQ_MAX> idt;
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_IDT_H_ */
