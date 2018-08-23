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
#include "types.h"

namespace cmds {

class CmdBase {
public:
    using CmdArgs = cstd::vector<cstd::string>;

    CmdBase() {}
    virtual ~CmdBase() {}

    // return created task_id, or 0 if no task created
    virtual u32 run(const CmdArgs& args) = 0;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_ */
