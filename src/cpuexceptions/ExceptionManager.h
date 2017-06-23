/**
 *   @file: ExceptionManager.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_
#define SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_

namespace cpuexceptions {

#include "types.h"

class ExceptionManager {
public:
    static ExceptionManager &instance();
    void on_exception(u8 exception_no, u64 error_code) const;

private:
    ExceptionManager() {}
    static ExceptionManager _instance;
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_EXCEPTIONMANAGER_H_ */
