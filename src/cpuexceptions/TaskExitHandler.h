/**
 *   @file: TaskExitHandler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_TASKEXITHANDLER_H_
#define SRC_CPUEXCEPTIONS_TASKEXITHANDLER_H_

#include "ExceptionHandler.h"

namespace cpuexceptions {

/**
 * @brief   Handler for reserved cpu exception 15 that seems to work when raised manually with "int $15"
 */
class TaskExitHandler: public ExceptionHandler {
public:
    s16 handled_exception_no() override;
    cpu::CpuState* on_exception(cpu::CpuState* cpu_state) override;
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_TASKEXITHANDLER_H_ */
