/**
 *   @file: ExceptionManager.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_
#define SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_

#include "CpuState.h"


namespace cpuexceptions {

class ExceptionManager {
public:
    static ExceptionManager &instance();
    ExceptionManager operator=(const ExceptionManager&) = delete;
    ExceptionManager operator=(ExceptionManager&&) = delete;

    cpu::CpuState* on_exception(u8 exception_no, cpu::CpuState* cpu_state) const;

private:
    ExceptionManager() {}
    static ExceptionManager _instance;
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_ */
