/**
 *   @file: cp.cpp
 *
 *   @date: Oct 18, 2017
 * @author: Mateusz Midor
 */

#include "cp.h"
#include "ustd.h"

using namespace ustd;
using namespace filesystem;
namespace cmds {

void cp::run() {
    if (env->cmd_args.size() < 3) {
        env->printer->format("cp: please specify src and dst paths\n");
        return;
    }

    string from = make_absolute_filename(env->cmd_args[1]);
    string to = make_absolute_filename(env->cmd_args[2]);
    if (!env->vfs_manager.copy_entry(from, to))
        env->printer->format("cp: couldn't copy entry '%' to '%'\n", from, to);
}
} /* namespace cmds */
