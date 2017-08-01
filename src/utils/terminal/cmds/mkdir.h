/**
 *   @file: mkdir.h
 *
 *   @date: Aug 1, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_MKDIR_H_
#define SRC_UTILS_TERMINAL_CMDS_MKDIR_H_

#include "CmdBase.h"

namespace cmds {

class mkdir: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_MKDIR_H_ */
