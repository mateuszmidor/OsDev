/**
 *   @file: rm.cpp
 *
 *   @date: Jul 28, 2017
 * @author: Mateusz Midor
 */

#include "rm.h"
#include "kstd.h"
#include "VolumeFat32.h"
#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;
namespace cmds {

void rm::run() {
    string path = env->command_line;
    string absolute_path;

    if (env->cwd.back() == '/')
        absolute_path = format("%%", env->cwd, path);
    else
        absolute_path = format("%/%", env->cwd, path);

    if (!env->volume->delete_entry(absolute_path))
        env->printer->format("rm: unable to remove %\n", absolute_path);
}
} /* namespace cmds */
