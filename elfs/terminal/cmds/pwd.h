/**
 *   @file: pwd.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_PWD_H_
#define SRC_UTILS_TERMINAL_CMDS_PWD_H_

#include "CmdBase.h"

namespace cmds {

class pwd : public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_PWD_H_ */
