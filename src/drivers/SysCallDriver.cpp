/**
 *   @file: SysCallDriver.cpp
 *
 *   @date: Aug 31, 2017
 * @author: Mateusz Midor
 */

#include "SysCallDriver.h"
#include "TaskManager.h"

using namespace hardware;
using namespace multitasking;

namespace drivers {

s16 SysCallDriver::handled_interrupt_no() {
    return Interrupts::SysCall;
}

hardware::CpuState* SysCallDriver::on_interrupt(hardware::CpuState* cpu_state) {
    // reschedule new task
//    TaskManager& mngr = TaskManager::instance();
//    return mngr.schedule(cpu_state);
    return cpu_state;
}

} /* namespace drivers */
