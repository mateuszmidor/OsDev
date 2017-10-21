/**
 *   @file: tail.h
 *
 *   @date: Aug 2, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_TAIL_H_
#define SRC_UTILS_TERMINAL_CMDS_TAIL_H_

#include "CmdBase.h"

namespace cmds {

class tail: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_TAIL_H_ */
