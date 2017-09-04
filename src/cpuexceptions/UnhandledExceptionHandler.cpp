/**
 *   @file: UnhandledExceptionHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "UnhandledExceptionHandler.h"
#include "KernelLog.h"
#include "TaskManager.h"

using multitasking::TaskManager;
using hardware::CpuState;
using utils::KernelLog;

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
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    klog.format("\nCPU EXCEPTION: %(%) by \"%\" [%], error %. KILLING\n",
                EXCEPTION_NAMES[exception_no],
                exception_no,
                current->name.c_str(),
                current->is_user_space ? "User" : "Kernel",
                cpu_state->error_code);

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
