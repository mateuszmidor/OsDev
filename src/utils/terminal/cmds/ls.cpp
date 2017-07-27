/**
 *   @file: ls.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "ls.h"

using namespace filesystem;
namespace cmds {

void ls::run() {
    if (env->volumes.empty())
        env->printer->format("No volumes installed\n");


    OnEntryFound on_entry = [&](const SimpleDentryFat32& e) -> bool {
        if (e.is_directory)
            env->printer->format("[%]\n", e.name);
        else
            env->printer->format("% - %B\n", e.name, e.size);

        return true;
    };

    VolumeFat32* v = env->volume;
    SimpleDentryFat32 e;
    if (v->get_entry(env->cwd, e))
        v->enumerate_directory_entry(e, on_entry);
}
} /* namespace cmds */
