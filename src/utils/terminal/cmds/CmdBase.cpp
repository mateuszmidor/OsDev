/**
 *   @file: CmdBase.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "CmdBase.h"

namespace cmds {

CmdBase::CmdBase(u64 arg) {
    env = (terminal::TerminalEnv*)arg;
}

/**
 * @brief   Construct absolute filename from current working directory + provided "relative_filename"
 */
kstd::string CmdBase::make_absolute_filename(const kstd::string& relative_filename) const {
    // check if relative_filename specified at all
    if (relative_filename.empty())
        return env->cwd;

    // check if relative_filename starts with a slash, which means it is actually an absolute filename
    if (relative_filename[0] == '/')
        return relative_filename;

    // make absolute filename from current working directory + relative filename
    return env->cwd + "/" + relative_filename;
}
} /* namespace cmds */
