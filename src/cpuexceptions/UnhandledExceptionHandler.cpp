/**
 *   @file: UnhandledExceptionHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "UnhandledExceptionHandler.h"
#include "ScreenPrinter.h"
#include "TaskManager.h"

using cpu::CpuState;
namespace cpuexceptions {

const char* UnhandledExceptionHandler::EXCEPTION_NAMES[32] = {
        "Divide-by-zero Error",
        "Debug",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Security Exception",
        "Reserved"
};
/**
 * Constructor.
 * This is generic handler so you can set the exception_no that it handles so it knows what it handles :)
 */
UnhandledExceptionHandler::UnhandledExceptionHandler(u8 exception_no) : exception_no(exception_no) {
}

s16 UnhandledExceptionHandler::handled_exception_no() {
    return exception_no;
}

CpuState* UnhandledExceptionHandler::on_exception(CpuState* cpu_state) {
    ScreenPrinter &printer = ScreenPrinter::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    printer.format("\nUNHANDLED CPU EXCEPTION: %(%) by \"%\", error code %. KILLING\n",
            EXCEPTION_NAMES[exception_no], exception_no, current->name.c_str(), cpu_state->error_code);

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
