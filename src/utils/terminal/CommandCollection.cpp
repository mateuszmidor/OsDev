/**
 *   @file: CommandCollection.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "CommandCollection.h"
#include <algorithm>

using namespace kstd;
using namespace multitasking;

namespace terminal {

TaskPtr CommandCollection::get(const kstd::string& cmd_name) {
    auto filt = [&cmd_name](const Command& cmd) { return cmd.name == cmd_name; };
    auto found = std::find_if(commands.begin(), commands.end(), filt);
    if (found != commands.end())
        return found->task;
    else
        return TaskPtr();
}

/**
 * @brief   Match known commands against name patter
 * @param   pattern Command name beginning
 * @return  {false, name} if single command found
 *          {true, name_list} if multiple commands found
 */
std::tuple<bool, string> CommandCollection::filter(const string& pattern) {
    kstd::vector<string> found;
    auto filt = [&pattern](const Command& c) { return c.name.find(pattern) != string::npos; };
    for (const Command& c : commands)
        if (c.name.find(pattern) == 0)
            found.push_back(c.name);

    if (found.empty())
        return std::make_tuple(false, pattern);
    else if (found.size() == 1)
        return std::make_tuple(false, found.back());
    else
        return std::make_tuple(true, kstd::join_string(" ", found));
}

void CommandCollection::install(const string name, TaskPtr task) {
    commands.push_back(Command{name, task});
}

} /* namespace terminal */
