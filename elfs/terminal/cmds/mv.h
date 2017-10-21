/**
 *   @file: mv.h
 *
 *   @date: Aug 1, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_MV_H_
#define SRC_UTILS_TERMINAL_CMDS_MV_H_

#include "CmdBase.h"

namespace cmds {

class mv: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_MV_H_ */
