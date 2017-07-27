/**
 *   @file: pwd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "pwd.h"

namespace cmds {

void pwd::run() {
    if (env->volumes.empty())
        env->printer->format("No volumes installed\n");
    else
        env->printer->format("%%\n", env->volume->get_label(), env->cwd);
}
} /* namespace cmds */
