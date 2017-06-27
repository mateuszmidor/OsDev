/**
 *   @file: ExceptionManager.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#include "ExceptionManager.h"
#include "ScreenPrinter.h"

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
    printer.format("CPU EXCEPTION %, error code %\n", exception_no, cpu_state->error_code);

    if (exception_no == 0) cpu_state->rcx = 1; // fix zero division

    return cpu_state;
}

} /* namespace cpuexceptions */
