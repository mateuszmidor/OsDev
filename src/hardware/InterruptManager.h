/**
 *   @file: InterrupManager.h
 *
 *   @date: Jun 19, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_INTERRUPTMANAGER_H_
#define SRC_INTERRUPTMANAGER_H_

#include <functional>
#include "Port.h"
#include "Idt.h"
#include "CpuState.h"

namespace hardware {


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

    Idt idt;
    hardware::Port8bitSlow pic_master_cmd   {0x20};
    hardware::Port8bitSlow pic_slave_cmd    {0xA0};
    hardware::Port8bitSlow pic_master_data  {0x21};
    hardware::Port8bitSlow pic_slave_data   {0xA1};

    InterruptManager() {}
    static hardware::CpuState* on_interrupt(u8 interrupt_no, hardware::CpuState* cpu_state);
    void ack_interrupt_handled(u8 interrupt_no);
    void config_interrupts();
    void setup_programmable_interrupt_controllers();

    InterruptHandler interrupt_handler = [](u8, hardware::CpuState* state) { return state; };
    ExceptionHandler exception_handler = [](u8, hardware::CpuState* state) { return state; };

    static const u8 SLAVE_PIC_IRQ_OFFSET = 8;
};
}   // namespace hardware

#endif /* SRC_INTERRUPTMANAGER_H_ */
