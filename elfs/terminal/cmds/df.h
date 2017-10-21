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
#include "TerminalEnv.h"

#include "CmdBase.h"

namespace cmds {

class df: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
    void print_hdd_info(drivers::AtaDevice& hdd);
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_DF_H_ */
