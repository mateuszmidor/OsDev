/**
 *   @file: trunc.h
 *
 *   @date: Aug 2, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_TRUNC_H_
#define SRC_UTILS_TERMINAL_CMDS_TRUNC_H_

#include "CmdBase.h"

namespace cmds {

class trunc: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_TRUNC_H_ */
