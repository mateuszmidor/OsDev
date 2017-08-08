/**
 *   @file: trunc.cpp
 *
 *   @date: Aug 2, 2017
 * @author: Mateusz Midor
 */

#include "trunc.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void trunc::run() {
    if (env->volumes.empty()) {
        env->printer->format("cat: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 3) {
        env->printer->format("cat: please specify file name and desired size in bytes\n");
        return;
    }

    string filename = env->cwd + "/" + env->cmd_args[1];
    u32 new_size = kstd::str_to_long(env->cmd_args[2].c_str());
    VolumeFat32* v = env->volume;
    auto e = v->get_entry(filename);
    if (!e) {
        env->printer->format("trunc: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory) {
        env->printer->format("trunc: % is a directory\n", filename);
        return;
    }

    v->trunc_file_entry(e, new_size);
}
} /* namespace cmds */
