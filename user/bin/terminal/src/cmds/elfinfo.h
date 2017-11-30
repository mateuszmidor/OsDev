/**
 *   @file: elfinfo.h
 *
 *   @date: Sep 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_ELFINFO_H_
#define SRC_UTILS_TERMINAL_CMDS_ELFINFO_H_

#include "CmdBase.h"

namespace cmds {

class elfinfo: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run(bool run_in_bg = false) override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_ELFINFO_H_ */
