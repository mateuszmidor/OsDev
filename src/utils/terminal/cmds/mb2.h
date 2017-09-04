/**
 *   @file: mb2.h
 *
 *   @date: Sep 4, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_MB2_H_
#define SRC_UTILS_TERMINAL_CMDS_MB2_H_

#include "CmdBase.h"

namespace cmds {

class mb2: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_MB2_H_ */
