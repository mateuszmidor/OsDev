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
#include "CpuState.h"
#include "InterruptNumbers.h"

namespace hardware {

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

using InterruptHandler = std::function<hardware::CpuState*(u8 interrupt_no, hardware::CpuState* cpu_state)>;
using ExceptionHandler = std::function<hardware::CpuState*(u8 exception_no, hardware::CpuState* cpu_state)>;

class InterruptManager {
public:
    static InterruptManager& instance();
    InterruptManager operator=(const InterruptManager&) = delete;
    InterruptManager operator=(InterruptManager&&) = delete;

    void config_and_activate_exceptions_and_interrupts();
    void set_interrupt_handler(const InterruptHandler &h);
    void set_exception_handler(const ExceptionHandler &h);

    u16 disable_interrupts();
    void enable_interrupts(u16 mask);

private:
    static InterruptManager _instance;
    hardware::Port8bitSlow pic_master_cmd   {0x20};
    hardware::Port8bitSlow pic_slave_cmd    {0xA0};
    hardware::Port8bitSlow pic_master_data  {0x21};
    hardware::Port8bitSlow pic_slave_data   {0xA1};

    InterruptManager() {}
    static hardware::CpuState* on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state);
    void ack_interrupt_handled(u8 interrupt_no);
    void config_interrupts();
    void setup_interrupt_descriptor_table();
    IdtEntry make_entry(u64 pointer);
    void setup_programmable_interrupt_controllers();
    void install_interrupt_descriptor_table();

    std::array<IdtEntry, Interrupts::IRQ_MAX> idt;
    InterruptHandler interrupt_handler = [](u8, hardware::CpuState* state) { return state; };
    ExceptionHandler exception_handler = [](u8, hardware::CpuState* state) { return state; };

    static const u8 SLAVE_PIC_IRQ_OFFSET = 8;
};
}   // namespace hardware
#endif /* SRC_INTERRUPTMANAGER_H_ */
