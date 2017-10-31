/**
 *   @file: CommandCollection.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_
#define SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_

#include <tuple>
#include "ustd.h"
#include "CmdBase.h"

namespace terminal {

struct Command {
    ustd::string        name;
    cmds::CmdBase*      task;

    Command(const ustd::string& name, cmds::CmdBase* task): name(name), task(task) {
    }
};

class CommandCollection {
public:
    cmds::CmdBase* get(const ustd::string& cmd_name) const;
    void install(const ustd::string name, cmds::CmdBase* cmd);

private:
    ustd::vector<Command> commands;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_COMMANDCOLLECTION_H_ */
