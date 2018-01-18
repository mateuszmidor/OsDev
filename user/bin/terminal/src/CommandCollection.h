/**
 *   @file: CommandCollection.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_
#define SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_

#include <tuple>
#include "CmdBase.h"

namespace terminal {

struct Command {
    cstd::string        name;
    cmds::CmdBase*      task;

    Command(const cstd::string& name, cmds::CmdBase* task): name(name), task(task) {
    }
};

class CommandCollection {
public:
    cmds::CmdBase* get(const cstd::string& cmd_name) const;
    void install(const cstd::string& name, cmds::CmdBase* cmd);

private:
    cstd::vector<Command> commands;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_ */
