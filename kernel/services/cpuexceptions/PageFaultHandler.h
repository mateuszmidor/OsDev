/**
 *   @file: PageFaultHandler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_
#define SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_

#include "types.h"
#include "ExceptionHandler.h"

namespace cpuexceptions {

class PageFaultHandler: public ExceptionHandler {
    s16 handled_exception_no() override;
    hardware::CpuState* on_exception(hardware::CpuState* cpu_state) override;
    size_t get_faulty_address();

    static const char* PF_REASON[];
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_ */
