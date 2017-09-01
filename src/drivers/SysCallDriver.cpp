/**
 *   @file: SysCallDriver.cpp
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#include "SysCallDriver.h"
#include "TaskManager.h"
#include "KernelLog.h"

using utils::KernelLog;
using namespace hardware;
using namespace multitasking;

namespace drivers {

s16 SysCallDriver::handled_interrupt_no() {
    return Interrupts::SysCall;
}

hardware::CpuState* SysCallDriver::on_interrupt(hardware::CpuState* cpu_state) {
    TaskManager& mngr = TaskManager::instance();
    switch (cpu_state->rax) {
    case 1: // sys_exit
        return task_exit();

    default:
        return mngr.schedule(cpu_state);
    }
}

hardware::CpuState* SysCallDriver::task_exit() {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    klog.format("[% task \"%\" exits]\n\n", current->is_user_space ? "User" : "Kernel", current->name.c_str());
    return mngr.kill_current_task();
}

} /* namespace drivers */
