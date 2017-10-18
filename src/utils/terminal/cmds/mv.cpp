/**
 *   @file: mv.cpp
 *
 *   @date: Aug 1, 2017
 * @author: Mateusz Midor
 */

#include "mv.h"
#include "kstd.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void mv::run() {
    if (env->cmd_args.size() < 3) {
        env->printer->format("mv: please specify src and dst paths\n");
        return;
    }

    string from = make_absolute_filename(env->cmd_args[1]);
    string to = make_absolute_filename(env->cmd_args[2]);
    if (!env->vfs_manager.move_entry(from, to))
        env->printer->format("mv: couldn't move entry from '%' to '%'\n", from, to);
}
} /* namespace cmds */
