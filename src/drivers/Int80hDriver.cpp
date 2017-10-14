/**
 *   @file: Int80hDriver.cpp
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#include "Int80hDriver.h"

#include "TaskManager.h"
#include "KernelLog.h"

using utils::KernelLog;
using namespace hardware;
using namespace multitasking;

namespace drivers {

s16 Int80hDriver::handled_interrupt_no() {
    return Interrupts::Int80h;
}

hardware::CpuState* Int80hDriver::on_interrupt(hardware::CpuState* cpu_state) {
    TaskManager& mngr = TaskManager::instance();
    switch (cpu_state->rax) {
    case 1: // sys_exit
        return task_exit(cpu_state->rbx);

    default:
        return mngr.schedule(cpu_state);
    }
}

hardware::CpuState* Int80hDriver::task_exit(u64 exit_code) {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    const Task& current = mngr.get_current_task();
    klog.format("[% task \"%\" exits with code %]\n\n", current.is_user_space ? "User" : "Kernel", current.name.c_str(), exit_code);
    return mngr.kill_current_task();
}

} /* namespace drivers */
