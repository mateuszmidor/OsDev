/**
 *   @file: ExceptionManager.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#include "ExceptionManager.h"
#include "ScreenPrinter.h"
#include "TaskManager.h"

using namespace cpu;

namespace cpuexceptions {

ExceptionManager ExceptionManager::_instance;


ExceptionManager &ExceptionManager::instance() {
    return _instance;
}

// this should dispatch different exceptions to individual exception handlers
// but lets keep it simple for now
CpuState* ExceptionManager::on_exception(u8 exception_no, CpuState* cpu_state) const {
    ScreenPrinter &printer = ScreenPrinter::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    printer.format("\nCPU EXCEPTION % by \"%\", error code %. KILLING\n",
            exception_no, current->name.c_str(), cpu_state->error_code);

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
