/**
 *   @file: cp.h
 *
 *   @date: Oct 18, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CP_H_
#define SRC_UTILS_TERMINAL_CMDS_CP_H_

#include "CmdBase.h"

namespace cmds {

class cp: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CP_H_ */
