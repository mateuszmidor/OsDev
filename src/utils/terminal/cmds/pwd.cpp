/**
 *   @file: pwd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "pwd.h"

namespace cmds {

void pwd::run() {
    env->printer->format("%\n", env->cwd);
}
} /* namespace cmds */
