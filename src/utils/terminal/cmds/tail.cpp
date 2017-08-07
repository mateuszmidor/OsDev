/**
 *   @file: tail.cpp
 *
 *   @date: Aug 2, 2017
 * @author: Mateusz Midor
 */

#include "tail.h"
#include "kstd.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void tail::run() {
    if (env->volumes.empty()) {
        env->printer->format("tail: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("tail: please specify file name\n");
        return;
    }

    string filename = env->cwd + "/" + env->cmd_args[1];
    VolumeFat32* v = env->volume;
    Fat32Entry e;
    if (!v->get_entry(filename, e)) {
        env->printer->format("tail: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory) {
        env->printer->format("tail: % is a directory\n", filename);
        return;
    }

    const u32 MAX_CHARS = 90;
    u32 position = e.size > MAX_CHARS ? e.size - MAX_CHARS : 0;
    v->seek_file_entry(e, position);
    const u32 BUFF_SIZE = 1025;
    static char buff[BUFF_SIZE]; // static to make sure recursive calls dont exhaust task stack
    u32 count;
    while ((count = v->read_file_entry(e, buff, BUFF_SIZE-1)) > 0) {
        buff[count] = '\0';
        env->printer->format("%", buff);
    }

    env->printer->format("\n");
}

} /* namespace cmds */
