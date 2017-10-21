/**
 *   @file: rm.h
 *
 *   @date: Jul 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_RM_H_
#define SRC_UTILS_TERMINAL_CMDS_RM_H_

#include "CmdBase.h"

namespace cmds {

class rm: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run();

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_RM_H_ */
