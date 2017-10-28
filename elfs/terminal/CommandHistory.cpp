/**
 *   @file: CommandHistory.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "CommandHistory.h"

using namespace ustd;
namespace terminal {

CommandHistory::CommandHistory() {
    history.push_back("");
}

void CommandHistory::append(const string& cmd) {
    if (!cmd.empty() && cmd != history.back()) {
        history.push_back(cmd);
        set_to_latest();
    }
}

void CommandHistory::set_to_latest() {
    index = history.size();
}

const string& CommandHistory::get_prev() {
    if (index > 0)
        index--;

    return history[index];
}

const string& CommandHistory::get_next() {
    if (index < history.size() -1) {
        index++;
    } else
        index = history.size() -1;

    return history[index];
}

} /* namespace terminal */
