/**
 *   @file: log.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_LOG_H_
#define SRC_UTILS_TERMINAL_CMDS_LOG_H_

#include "CmdBase.h"

namespace cmds {

class log: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_LOG_H_ */
