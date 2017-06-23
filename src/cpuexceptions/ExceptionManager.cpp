/**
 *   @file: ExceptionManager.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#include "ExceptionManager.h"
#include "ScreenPrinter.h"

namespace cpuexceptions {

ExceptionManager ExceptionManager::_instance;


ExceptionManager &ExceptionManager::instance() {
    return _instance;
}

// this should dispatch different exceptions to individual exception handlers
// but lets keep it simple for now
void ExceptionManager::on_exception(u8 exception_no, u64 error_code) const {
    ScreenPrinter &printer = ScreenPrinter::instance();
    printer.format("CPU EXCEPTION %, error code %\n", exception_no, error_code);
}

} /* namespace cpuexceptions */
