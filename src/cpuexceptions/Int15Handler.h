/**
 *   @file: Int15Handler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_INT15HANDLER_H_
#define SRC_CPUEXCEPTIONS_INT15HANDLER_H_

#include "ExceptionHandler.h"
#include "TaskManager.h"

namespace cpuexceptions {

/**
 * @brief   Handler for reserved cpu exception 15 that seems to work when raised manually with "int $15"
 */
class Int15Handler: public ExceptionHandler {
public:
    s16 handled_exception_no() override;
    cpu::CpuState* on_exception(cpu::CpuState* cpu_state) override;
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_INT15HANDLER_H_ */
