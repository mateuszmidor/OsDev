/**
 *   @file: UnhandledExceptionHandler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_
#define SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_

#include "CpuState.h"

namespace cpuexceptions {

/**
 * @brief   This is a generic exception handler; for exceptions that have no specific handler associated
 */
class UnhandledExceptionHandler {
public:
    static hardware::CpuState* on_exception(u8 exception_no, hardware::CpuState* cpu_state);

private:
    static const char* EXCEPTION_NAMES[];
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_ */
