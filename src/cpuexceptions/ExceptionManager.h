/**
 *   @file: ExceptionManager.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_
#define SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_

#include <array>
#include "InterruptNumbers.h"
#include "UnhandledExceptionHandler.h"

namespace cpuexceptions {

using ExceptionHandlerPtr = ExceptionHandler*;
class ExceptionManager {
public:
    static ExceptionManager &instance();
    ExceptionManager operator=(const ExceptionManager&) = delete;
    ExceptionManager operator=(ExceptionManager&&) = delete;

    void install_handler(ExceptionHandlerPtr handler);
    hardware::CpuState* on_exception(u8 exception_no, hardware::CpuState* cpu_state) const;

private:
    ExceptionManager();
    static ExceptionManager _instance;
    static UnhandledExceptionHandler unhandled_exception_handler;
    std::array<ExceptionHandlerPtr, hardware::Interrupts::EXC_MAX> handlers; // this array maps exception_no to handler
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_ */
