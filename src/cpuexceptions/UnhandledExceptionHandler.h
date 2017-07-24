/**
 *   @file: UnhandledExceptionHandler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_
#define SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_

#include "ExceptionHandler.h"

namespace cpuexceptions {

class UnhandledExceptionHandler: public ExceptionHandler {
public:
    UnhandledExceptionHandler(u8 exception_no);
    ~UnhandledExceptionHandler() override {}
    s16 handled_exception_no() override;
    hardware::CpuState* on_exception(hardware::CpuState* cpu_state) override;

private:
    u8 exception_no;
    static const char* EXCEPTION_NAMES[];
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_UNHANDLEDEXCEPTIONHANDLER_H_ */
