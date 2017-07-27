/**
 *   @file: cd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cd.h"
#include "kstd.h"
#include "VolumeFat32.h"
#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;

namespace cmds {

void cd::run(u64 arg) {
    env = (terminal::TerminalEnv*)arg;
    auto cmds = split_string<vector<string>>(env->command_line, ' ');
}

} /* namespace cmds */
