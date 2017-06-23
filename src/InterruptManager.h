/**
 *   @file: InterrupManager.h
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_INTERRUPTMANAGER_H_
#define SRC_INTERRUPTMANAGER_H_

#include <functional>
#include <array>
#include "types.h"
#include "Port.h"

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

using InterruptHandler = std::function<void(u8 interrupt_no)>;
using ExceptionHandler = std::function<void(u8 exception_no, u64 error_code)>;

class InterruptManager {
public:
    static InterruptManager& instance();
    void config_and_activate_exceptions_and_interrupts();
    void set_interrupt_handler(const InterruptHandler &h);
    void set_exception_handler(const ExceptionHandler &h);

private:
    static constexpr int MAX_INTERRUPT_COUNT = 256;
    static InterruptManager _instance;
    Port8bitSlow pic_master_cmd  {0x20};
    Port8bitSlow pic_slave_cmd   {0xA0};

    InterruptManager();
    static void on_interrupt(u8 interrupt_no, u64 error_code);
    void ack_interrupt_handled(u8 interrupt_no);
    void config_interrupts();
    void setup_interrupt_descriptor_table();
    IdtEntry make_entry(u64 pointer, u16 code_segment_selector = 8);
    void setup_programmable_interrupt_controllers();
    void install_interrupt_descriptor_table();

    std::array<IdtEntry, MAX_INTERRUPT_COUNT> idt;
    InterruptHandler interrupt_handler = [](u8) { /* do nothing */ };
    ExceptionHandler exception_handler = [](u8, u64) { /* do nothing */ };
};

#endif /* SRC_INTERRUPTMANAGER_H_ */
