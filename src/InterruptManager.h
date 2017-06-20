/**
 *   @file: InterrupManager.h
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_INTERRUPTMANAGER_H_
#define SRC_INTERRUPTMANAGER_H_

#include <functional>
#include "types.h"

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

/**
 * Struct to be loaded with lidt instruction
 */
struct IdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed)) ;

using InterruptHandler = std::function<void(u8 interrupt_no, u64 error_code)>;

class InterruptManager {
public:
    static void config_interrupts();
    static void set_handler(const InterruptHandler &h);

private:
    static void on_interrupt(u8 interrupt_no, u64 error_code);
    static void setup_interrupt_descriptor_table();
    static IdtEntry make_entry(u64 pointer, u16 code_segment_selector = 8);
    static void install_interrupt_descriptor_table();

    static IdtEntry idt[0x14];
    static InterruptHandler interrupt_handler;
};

#endif /* SRC_INTERRUPTMANAGER_H_ */
