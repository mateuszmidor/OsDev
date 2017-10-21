/**
 *   @file: lscpu.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_LSCPU_H_
#define SRC_UTILS_TERMINAL_CMDS_LSCPU_H_

#include "CmdBase.h"

namespace cmds {

class lscpu: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_LSCPU_H_ */
