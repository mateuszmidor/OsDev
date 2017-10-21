/**
 *   @file: rm.cpp
 *
 *   @date: Jul 28, 2017
 * @author: Mateusz Midor
 */

#include "rm.h"
#include "ustd.h"
#include "DriverManager.h"

using namespace ustd;
using namespace utils;
using namespace drivers;
using namespace filesystem;
namespace cmds {

void rm::run() {
    if (env->cmd_args.size() < 2) {
        env->printer->format("rm: please specify file name\n");
        return;
    }

    string filename = make_absolute_filename(env->cmd_args[1]);

    if (!env->vfs_manager.delete_entry(filename))
        env->printer->format("rm: unable to remove %\n", filename);
}
} /* namespace cmds */
