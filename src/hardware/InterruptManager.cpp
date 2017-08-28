/**
 *   @file: InterrupManager.cpp
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"
#include "InterruptManager.h"

// CPU exceptions
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

// PIC interrupts, shifted by IRQ_BASE
extern "C" void ignore_interrupt();
extern "C" void handle_interrupt_no_0x20();
extern "C" void handle_interrupt_no_0x21();
extern "C" void handle_interrupt_no_0x2C();
extern "C" void handle_interrupt_no_0x2E();
extern "C" void handle_interrupt_no_0x2F();

namespace hardware {

InterruptManager InterruptManager::_instance;


InterruptManager& InterruptManager::instance() {
    return _instance;
}

void InterruptManager::config_interrupts() {
    setup_programmable_interrupt_controllers();
    setup_interrupt_descriptor_table();
    install_interrupt_descriptor_table();
}

/**
 * @name    on_interrupt
 * @brief   Interrupt/CPU exception handler. This is called from interrupts.S
 * @param   interrupt_no Number of CPU exception/PIC interrupt (offset by IRQ_BASE) being handled
 * @param   cpu_state Pointer to the stack holding current cpu::CpuState struct
 * @return  Pointer to stack pointer holding destination cpu::CpuState struct (for task switching)
 * @note    STATIC FUNCTION
 */
hardware::CpuState* InterruptManager::on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state) {
    InterruptManager &mngr = InterruptManager::instance();
    hardware::CpuState* new_cpu_state;

    // if its PIC interrupt
    if (interrupt_no >= Interrupts::IRQ_BASE) {
        new_cpu_state = mngr.interrupt_handler(interrupt_no, cpu_state);
        mngr.ack_interrupt_handled(interrupt_no);
    }
    // if its CPU exception
    else
        new_cpu_state = mngr.exception_handler(interrupt_no, cpu_state);

    return new_cpu_state;
}

void InterruptManager::ack_interrupt_handled(u8 interrupt_no) {
    // send End Of Interrupt (confirm to PIC that interrupt has been handled)
    if (interrupt_no >= Interrupts::IRQ_BASE + SLAVE_PIC_IRQ_OFFSET)
        pic_slave_cmd.write(0x20);
    pic_master_cmd.write(0x20);
}

void InterruptManager::config_and_activate_exceptions_and_interrupts() {
    config_interrupts();
    __asm__("sti");
}

void InterruptManager::set_interrupt_handler(const InterruptHandler &h) {
    interrupt_handler = h;
}

void InterruptManager::set_exception_handler(const ExceptionHandler &h) {
    exception_handler = h;
}

void InterruptManager::setup_interrupt_descriptor_table() {
    // CPU exceptions
    idt[0x00] = make_entry((u64) (handle_exception_no_0x00)); // zero division
    idt[0x01] = make_entry((u64) (handle_exception_no_0x01));
    idt[0x02] = make_entry((u64) (handle_exception_no_0x02));
    idt[0x03] = make_entry((u64) (handle_exception_no_0x03)); // breakpoint
    idt[0x04] = make_entry((u64) (handle_exception_no_0x04));
    idt[0x05] = make_entry((u64) (handle_exception_no_0x05));
    idt[0x06] = make_entry((u64) (handle_exception_no_0x06));
    idt[0x07] = make_entry((u64) (handle_exception_no_0x07));
    idt[0x08] = make_entry((u64) (handle_exception_no_0x08));
    idt[0x09] = make_entry((u64) (handle_exception_no_0x09));
    idt[0x0A] = make_entry((u64) (handle_exception_no_0x0A));
    idt[0x0B] = make_entry((u64) (handle_exception_no_0x0B));
    idt[0x0C] = make_entry((u64) (handle_exception_no_0x0C));
    idt[0x0D] = make_entry((u64) (handle_exception_no_0x0D));
    idt[0x0E] = make_entry((u64) (handle_exception_no_0x0E));
    idt[0x0F] = make_entry((u64) (handle_exception_no_0x0F));
    idt[0x10] = make_entry((u64) (handle_exception_no_0x10));
    idt[0x11] = make_entry((u64) (handle_exception_no_0x11));
    idt[0x12] = make_entry((u64) (handle_exception_no_0x12));
    idt[0x13] = make_entry((u64) (handle_exception_no_0x13));
    idt[0x14] = make_entry((u64) (handle_exception_no_0x14));
    idt[0x1E] = make_entry((u64) (handle_exception_no_0x1E));

    // PIC interrupts, they start at IRQ_BASE = 0x20, defined in interrupts.S
    // first setup idt to ignore all interrupts
    for (u32 i = Interrupts::IRQ_BASE; i < Interrupts::IRQ_MAX; i++)
        idt[i] = make_entry((u64) (ignore_interrupt));

    // then handle just the interrupts of interest
    idt[Interrupts::Timer]          = make_entry((u64) (handle_interrupt_no_0x20));   // timer
    idt[Interrupts::Keyboard]       = make_entry((u64) (handle_interrupt_no_0x21));   // keyboard
    idt[Interrupts::Mouse]          = make_entry((u64) (handle_interrupt_no_0x2C));   // mouse
    idt[Interrupts::PrimaryAta]     = make_entry((u64) (handle_interrupt_no_0x2E));   // primary ata bus
    idt[Interrupts::SecondaryAta]   = make_entry((u64) (handle_interrupt_no_0x2F));   // secondary ata bus
}

IdtEntry InterruptManager::make_entry(u64 pointer) {
    IdtEntry e;

    e.gdt_code_segment_selector = Gdt::get_kernel_code_segment_selector();
    e.pointer_low = pointer & 0xFFFF;
    e.pointer_middle = (pointer >> 16) & 0xFFFF;
    e.pointer_high = (pointer >> 32) & 0xFFFFFFFF;
    e.options = IdtEntryOptions(true);
    e.always_0 = 0;

    return e;
}

void InterruptManager::setup_programmable_interrupt_controllers() {
    // save master and slave masks
    u8 master_mask = pic_master_data.read();
    u8 slave_mask = pic_slave_data.read();

    // start PIC initialization sequence
    pic_master_cmd.write(0x11);
    pic_slave_cmd.write(0x11);

    // setup IRQ BASE
    pic_master_data.write(Interrupts::IRQ_BASE);
    pic_slave_data.write(Interrupts::IRQ_BASE + SLAVE_PIC_IRQ_OFFSET);

    // tell master PIC that there is slave PIC at IRQ2 (0000 0100)
    pic_master_data.write(0x04);

    // tell slave PIC its cascade identity (0000 0010)
    pic_slave_data.write(0x02);

    // set PICs in 8086/88 (MCS-80/85) mode
    pic_master_data.write(0x01);
    pic_slave_data.write(0x01);

    // restore saved masks
    pic_master_data.write(master_mask);
    pic_slave_data.write(slave_mask);
}

void InterruptManager::install_interrupt_descriptor_table() {
    IdtSizeAddress idt_size_address;
    idt_size_address.size_minus_1 = sizeof(IdtEntry) * idt.size() - 1;
    idt_size_address.address = (u64) (idt.data());

    asm("lidt %0" : : "m" (idt_size_address));
}

/**
 * @brief   Disable all interrupts
 * @return  What interrupts were enabled before
 */
u16 InterruptManager::disable_interrupts() {
    u8 master_mask = pic_master_data.read();
    u8 slave_mask = pic_slave_data.read();
    pic_master_data.write(0xFF);
    pic_slave_data.write(0xFF);
    return (master_mask << 8) | slave_mask;
}

/**
 * @brief   Enable interrupts
 * @param   mask What interrupts to enable
 */
void InterruptManager::enable_interrupts(u16 mask) {
    u8 master_mask = mask >> 8;
    u8 slave_mask = mask & 0xFF;
    pic_master_data.write(master_mask);
    pic_slave_data.write(slave_mask);
}

} // namespace hardware
