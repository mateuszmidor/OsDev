/**
 *   @file: cat.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cat.h"
#include "kstd.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void cat::run() {
    if (env->volumes.empty()) {
        env->printer->format("cat: no volumes installed\n");
        return;
    }

//    auto cmds = kstd::split_string<vector<string>>(env->command_line, ' ');
//    if (cmds.size() < 2) {
//        env->printer->format("No filename passed to cat command\n");
//        return;
//    }

    string filename = env->cwd + "/" + env->command_line;//cmds[1];
    VolumeFat32* v = env->volume;
    SimpleDentryFat32 e;
    if (!v->get_entry(filename, e)) {
        env->printer->format("cat: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory) {
        env->printer->format("cat: % is a directory\n", filename);
        return;
    }

    const u32 SIZE = 1025;
    static char buff[SIZE]; // static to make sure recursive calls dont exhaust task stack
    u32 count;
    while ((count = v->read_file_entry(e, buff, SIZE-1)) > 0) {
        buff[count] = '\0';
        env->printer->format("%", buff);
    }

    env->printer->format("\n");
}
} /* namespace cmds */
