/**
 *   @file: elfrun.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_
#define SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_

#include "kstd.h"
#include "CmdBase.h"

namespace cmds {

class elfrun: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:

};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_ */
