/**
 *   @file: CommandCollection.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "CommandCollection.h"

using namespace cstd;
using namespace cmds;

namespace terminal {

CmdBase* CommandCollection::get(const string& cmd_name) const {
    for (const Command& cmd : commands)
        if (cmd.name == cmd_name)
            return cmd.task;

    return nullptr;
}

void CommandCollection::install(const string& name, CmdBase* task) {
    commands.emplace_back(name, task);
}

} /* namespace terminal */
