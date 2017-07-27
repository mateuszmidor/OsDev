/**
 *   @file: ls.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_LS_H_
#define SRC_UTILS_TERMINAL_CMDS_LS_H_

#include "CmdBase.h"

namespace cmds {

class ls: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_LS_H_ */
