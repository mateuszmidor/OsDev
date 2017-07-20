/**
 *   @file: TaskExitHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "TaskExitHandler.h"

#include "KernelLog.h"
#include "TaskManager.h"

using cpu::CpuState;
namespace cpuexceptions {

s16 TaskExitHandler::handled_exception_no() {
    return Interrupts::TaskExit;
}

/**
 * cpu exception 15 is temporarily used to signal that current task exits
 */
CpuState* TaskExitHandler::on_exception(CpuState* cpu_state) {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    klog.format(" [Task \"%\" exits] ", current->name.c_str());

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
