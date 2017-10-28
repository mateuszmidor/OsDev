/**
 *   @file: CommandCollection.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "CommandCollection.h"
#include <algorithm>

using namespace ustd;
using namespace cmds;
namespace terminal {

CmdBase* CommandCollection::get(const ustd::string& cmd_name) const {
    for (const Command& cmd : commands)
        if (cmd.name == cmd_name)
            return cmd.task;

    return nullptr;
}

void CommandCollection::install(const string name, CmdBase* task) {
    commands.emplace_back(name, task);
}

} /* namespace terminal */
