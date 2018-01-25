/**
 *   @file: time.cpp
 *
 *   @date: Jan 19, 2018
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "timeit.h"
#include "Cout.h"
#include "Timer.h"
#include "StringUtils.h"

using namespace cstd;
using namespace cstd::ustd;

namespace cmds {

void timeit::run(const CmdArgs& args, bool run_in_bg) {
    CmdArgs args_no_head = {args.begin() + 1, args.end()};
    if (args_no_head.empty()) {
        cout::format("timeit: please provide command to run\n");
        return;
    }

    Timer t;
    run_cmd(args_no_head);
    double dt = t.get_delta_seconds();
    string seconds = StringUtils::from_double(dt, 3);
    cout::format("execution took %s\n", seconds);
}

} /* namespace cmds */
