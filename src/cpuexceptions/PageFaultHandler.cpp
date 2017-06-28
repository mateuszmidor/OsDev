/**
 *   @file: PageFaultHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "PageFaultHandler.h"
#include "ScreenPrinter.h"
#include "TaskManager.h"

using cpu::CpuState;
namespace cpuexceptions {

s16 PageFaultHandler::handled_exception_no() {
    return 14;
}

/**
 * @brief   Handle page fault by printing message and killing offending task.
 *          In future it should conditionally allocate the page and resume task execution
 */
CpuState* PageFaultHandler::on_exception(CpuState* cpu_state) {
    u64 faulty_address;
    asm("mov %%cr2, %%rax;  mov %%rax, %0" : "=m"(faulty_address));

    ScreenPrinter& printer = ScreenPrinter::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    printer.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". KILLING] ",
            faulty_address, faulty_address /1024 / 1024,  faulty_address /1024 / 1024 / 1024, current->name.c_str());

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
