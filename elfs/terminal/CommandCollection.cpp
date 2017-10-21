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

/**
 * @brief   Match known commands against name patter
 * @param   pattern Command name beginning
 * @return  {false, name} if single command found
 *          {true, name_list} if multiple commands found
 */
std::tuple<bool, string> CommandCollection::filter(const string& pattern) {
    ustd::vector<string> found;
    for (const Command& c : commands)
        if (c.name.find(pattern) == 0)
            found.push_back(c.name);

    if (found.empty())
        return std::make_tuple(false, pattern);
    else if (found.size() == 1)
        return std::make_tuple(false, found.back());
    else
        return std::make_tuple(true, ustd::join_string(" ", found));
}

void CommandCollection::install(const string name, CmdBase* task) {
    commands.emplace_back(name, task);
}

} /* namespace terminal */
