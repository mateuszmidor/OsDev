/**
 *   @file: ExceptionHandler
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_EXCEPTIONHANDLER_H_
#define SRC_CPUEXCEPTIONS_EXCEPTIONHANDLER_H_

#include "CpuState.h"
#include "InterruptNumbers.h"

namespace cpuexceptions {

class ExceptionHandler {
public:
    ExceptionHandler() {}
    virtual ~ExceptionHandler() {}

    // number of cpu exception that this driver handles
    virtual s16 handled_exception_no() = 0;

    // if no task switching to be done, we should just return cpu_state
    virtual hardware::CpuState* on_exception(hardware::CpuState* cpu_state) = 0;
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_EXCEPTIONHANDLER_H_ */
