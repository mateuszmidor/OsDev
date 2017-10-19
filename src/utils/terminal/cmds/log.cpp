/**
 *   @file: log.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "log.h"

namespace cmds {

void log::run() {
    if (env->cmd_args.size() == 1) {
        env->printer->format("%\n", env->klog->get_text());
        return;
    }

    // given a parameter
    const kstd::string action = env->cmd_args[1];
    if (action == "-clear")
       env->klog->clear();
    else
       env->printer->format("log: invalid command %\n", action);


}
} /* namespace cmds */
