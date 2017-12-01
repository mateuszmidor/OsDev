/**
 *   @file: elfrun.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_
#define SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_

#include "ustd.h"
#include "CmdBase.h"

namespace cmds {

class elfrun: public CmdBase {
public:
    void run(const CmdArgs& args, bool run_in_bg = false) override;

};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_ */
