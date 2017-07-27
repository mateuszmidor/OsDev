/**
 *   @file: cd.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CD_H_
#define SRC_UTILS_TERMINAL_CMDS_CD_H_

#include "AtaDriver.h"
#include "TerminalEnv.h"

namespace cmds {

class cd {
public:
    void run(u64 arg);

private:
    terminal::TerminalEnv* env;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CD_H_ */
