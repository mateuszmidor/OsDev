/**
 *   @file: ExceptionManager.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_
#define SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_

#include <array>
#include <memory>
#include "InterruptNumbers.h"
#include "ExceptionHandler.h"

namespace cpuexceptions {

using ExceptionHandlerPtr = std::shared_ptr<ExceptionHandler>;
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
    std::array<ExceptionHandlerPtr, hardware::Interrupts::EXC_MAX> handlers; // this array maps exception_no to handler
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_ */
