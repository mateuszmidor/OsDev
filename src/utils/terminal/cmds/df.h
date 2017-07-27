/**
 *   @file: df.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_DF_H_
#define SRC_UTILS_TERMINAL_CMDS_DF_H_

#include "types.h"
#include "AtaDriver.h"
#include "ScrollableScreenPrinter.h"

namespace cmds {

class df {
public:
    void run(u64 arg);

private:
    void print_hdd_info(drivers::AtaDevice& hdd);

    utils::ScrollableScreenPrinter* p;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_DF_H_ */
