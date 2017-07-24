/**
 *   @file: PageFaultHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "PageFaultHandler.h"
#include "KernelLog.h"
#include "TaskManager.h"

using utils::KernelLog;
using hardware::CpuState;
using multitasking::TaskManager;

namespace cpuexceptions {

s16 PageFaultHandler::handled_exception_no() {
    return hardware::Interrupts::PageFault;
}

/**
 * @brief   Handle page fault by printing message and killing offending task.
 *          In future it should conditionally allocate the page and resume task execution
 */
CpuState* PageFaultHandler::on_exception(CpuState* cpu_state) {
    u64 faulty_address;
    asm("mov %%cr2, %%rax;  mov %%rax, %0" : "=m"(faulty_address));

    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". KILLING] ",
                faulty_address, faulty_address /1024 / 1024,  faulty_address /1024 / 1024 / 1024, current->name.c_str());

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
