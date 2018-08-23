/**
 *   @file: time.h
 *
 *   @date: Jan 19, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_BIN_TERMINAL_SRC_CMDS_TIMEIT_H_
#define USER_BIN_TERMINAL_SRC_CMDS_TIMEIT_H_

#include <functional>
#include "CmdBase.h"

namespace cmds {

class timeit: public CmdBase {
    using RunCmdFunc = std::function<void(const CmdArgs&)>;
    RunCmdFunc    run_cmd;

public:
    timeit(const RunCmdFunc& run_cmd) : run_cmd(run_cmd) {}
    u32 run(const CmdArgs& args) override;
};

} /* namespace cmds */

#endif /* USER_BIN_TERMINAL_SRC_CMDS_TIMEIT_H_ */
