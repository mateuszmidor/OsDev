/**
 *   @file: mkdir.cpp
 *
 *   @date: Aug 1, 2017
 * @author: Mateusz Midor
 */

#include "mkdir.h"

using namespace kstd;
namespace cmds {

void mkdir::run() {
    if (env->volumes.empty()) {
        env->printer->format("mkdir: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("mkdir: please specify dir name\n");
        return;
    }

    string dirname = env->cwd + "/" + env->cmd_args[1];

    if (!env->volume->create_entry(dirname, true)) {
        env->printer->format("mkdir: could not create directory '%'\n", dirname);
    }
}

} /* namespace cmds */
