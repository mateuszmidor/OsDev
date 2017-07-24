/**
 *   @file: ExceptionManager.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#include "ExceptionManager.h"
#include "UnhandledExceptionHandler.h"
#include "TaskManager.h"

using namespace hardware;

namespace cpuexceptions {

ExceptionManager ExceptionManager::_instance;


ExceptionManager &ExceptionManager::instance() {
    return _instance;
}

ExceptionManager::ExceptionManager() {
    for (u16 i = 0; i < handlers.size(); i++)
        handlers[i] = std::make_shared<UnhandledExceptionHandler>(i);
}

void ExceptionManager::install_handler(ExceptionHandlerPtr handler) {
    auto exception_no = handler->handled_exception_no();
    handlers[exception_no] = handler;
}

CpuState* ExceptionManager::on_exception(u8 exception_no, CpuState* cpu_state) const {
    return handlers[exception_no]->on_exception(cpu_state);
}

} /* namespace cpuexceptions */
