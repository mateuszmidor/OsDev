/**
 *   @file: Int15Handler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "Int15Handler.h"
#include "ScreenPrinter.h"
#include "TaskManager.h"

using cpu::CpuState;
namespace cpuexceptions {

s16 Int15Handler::handled_exception_no() {
    return 15;
}

/**
 * cpu exception 15 is temporarily used to signal that current task exits
 */
CpuState* Int15Handler::on_exception(CpuState* cpu_state) {
    ScreenPrinter& printer = ScreenPrinter::instance();
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();
    printer.format(" [Task \"%\" exits] ", current->name.c_str());

    return mngr.kill_current_task();
}

} /* namespace cpuexceptions */
