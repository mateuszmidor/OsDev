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

} /* namespace cmds */
