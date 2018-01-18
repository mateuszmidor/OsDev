/**
 *   @file: CmdBase.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_
#define SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_

#include "Vector.h"
#include "String.h"

namespace cmds {

class CmdBase {
public:
    using CmdArgs = cstd::vector<cstd::string>;

    CmdBase() {}
    virtual ~CmdBase() {}
    virtual void run(const CmdArgs& args, bool run_in_bg = false) = 0;

};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_ */
