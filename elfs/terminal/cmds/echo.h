/**
 *   @file: echo.h
 *
 *   @date: Jul 31, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_ECHO_H_
#define SRC_UTILS_TERMINAL_CMDS_ECHO_H_

#include "CmdBase.h"

namespace cmds {

class echo: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_ECHO_H_ */
