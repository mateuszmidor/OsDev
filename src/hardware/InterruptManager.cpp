/**
 *   @file: InterrupManager.cpp
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#include "InterruptManager.h"


namespace hardware {

InterruptManager InterruptManager::_instance;

/**
 * @brief   This "C" style function simply forwards calls from interrupts.S to InterruptManager instance
 */
extern "C" hardware::CpuState* on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state) {
    InterruptManager& mngr = InterruptManager::instance();
    return mngr.on_interrupt(interrupt_no, cpu_state);
}

InterruptManager& InterruptManager::instance() {
    return _instance;
}

void InterruptManager::config_interrupts() {
    setup_programmable_interrupt_controllers();
    idt.reinstall_idt();
}

/**
 * @name    on_interrupt
 * @brief   Interrupt/CPU exception handler
 * @param   interrupt_no Number of CPU exception/PIC interrupt (offset by IRQ_BASE) being handled
 * @param   cpu_state Pointer to the stack holding current cpu::CpuState struct
 * @return  Pointer to stack pointer holding destination cpu::CpuState struct (for task switching)
 */
hardware::CpuState* InterruptManager::on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state) {
    hardware::CpuState* new_cpu_state;

    // if its PIC interrupt
    if (interrupt_no >= Interrupts::IRQ_BASE) {
        new_cpu_state = interrupt_handler(interrupt_no, cpu_state);
        ack_interrupt_handled(interrupt_no);
    }
    // if its CPU exception
    else
        new_cpu_state = exception_handler(interrupt_no, cpu_state);

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
    asm("sti");
}

void InterruptManager::set_interrupt_handler(const InterruptHandler &h) {
    interrupt_handler = h;
}

void InterruptManager::set_exception_handler(const ExceptionHandler &h) {
    exception_handler = h;
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
