/**
 *   @file: Int80hDriver.cpp
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#include "Int80hDriver.h"
#include "SysCallNumbers.h"
#include "TaskManager.h"
#include "KernelLog.h"

using logging::KernelLog;
using namespace hardware;
using namespace middlespace;
using namespace multitasking;
namespace drivers {

s16 Int80hDriver::handled_interrupt_no() {
    return Interrupts::Int80h;
}

hardware::CpuState* Int80hDriver::on_interrupt(hardware::CpuState* cpu_state) {
    TaskManager& mngr = TaskManager::instance();
    Int80hSysCallNumbers syscall = (Int80hSysCallNumbers)cpu_state->rax;

    switch (syscall) {
    case Int80hSysCallNumbers::EXIT:
        return task_exit(cpu_state->rbx);

    case Int80hSysCallNumbers::EXIT_GROUP:
        return task_exit_group(cpu_state->rbx);

    case Int80hSysCallNumbers::NANOSLEEP:
        cpu_state->rax = 0; // return value
        return mngr.sleep_current_task(cpu_state, cpu_state->rbx / 1000 / 1000);

    default:
        return cpu_state;
    }
}

hardware::CpuState* Int80hDriver::task_exit(u64 exit_code) {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    const Task& current = mngr.get_current_task();
    klog.format("[% task \"%\" exits with code %]\n", current.is_user_space ? "User" : "Kernel", current.name.c_str(), exit_code);
    return mngr.kill_current_task();
}

hardware::CpuState* Int80hDriver::task_exit_group(u64 exit_code) {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    const Task& current = mngr.get_current_task();
    klog.format("[% task group \"%\" exits with code %]\n", current.is_user_space ? "User" : "Kernel", current.name.c_str(), exit_code);
    return mngr.kill_current_task_group();
}

} /* namespace drivers */
