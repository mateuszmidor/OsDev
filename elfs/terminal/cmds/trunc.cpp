/**
 *   @file: trunc.cpp
 *
 *   @date: Aug 2, 2017
 * @author: Mateusz Midor
 */

#include "trunc.h"

using namespace ustd;
using namespace filesystem;
namespace cmds {

void trunc::run() {
    if (env->cmd_args.size() < 3) {
        env->printer->format("cat: please specify file name and desired size in bytes\n");
        return;
    }

    string filename = make_absolute_filename(env->cmd_args[1]);
    u32 new_size = ustd::str_to_long(env->cmd_args[2].c_str());

    VfsEntryPtr e = env->vfs_manager.get_entry(filename);
    if (!e) {
        env->printer->format("trunc: file % doesnt exist\n", filename);
        return;
    }

    if (e->is_directory()) {
        env->printer->format("trunc: % is a directory\n", filename);
        return;
    }

    e->truncate(new_size);
}
} /* namespace cmds */
