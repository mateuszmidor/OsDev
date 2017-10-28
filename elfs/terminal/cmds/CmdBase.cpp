/**
 *   @file: CmdBase.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "CmdBase.h"

namespace cmds {

CmdBase::CmdBase(terminal::TerminalEnv* arg) {
    env = arg;
}

} /* namespace cmds */
