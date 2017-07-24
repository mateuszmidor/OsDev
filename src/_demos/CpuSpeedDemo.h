/**
 *   @file: CpuSpeedDemo.h
 *
 *   @date: Jul 24, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_CPUSPEEDDEMO_H_
#define SRC__DEMOS_CPUSPEEDDEMO_H_

#include "TaskManager.h"

namespace demos {

class CpuSpeedDemo {
public:
    void run();

private:
    u64 rtdsc();
};

} /* namespace demos */

#endif /* SRC__DEMOS_CPUSPEEDDEMO_H_ */
