/**
 *   @file: Terminal.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINAL_H_
#define SRC_UTILS_TERMINAL_TERMINAL_H_

#include "KeyboardScanCodeSet.h"
#include "TerminalEnv.h"
#include "CommandHistory.h"
#include "CommandCollection.h"
//#include "TaskFactory.h"
#include "ustd.h"

namespace terminal {

class Terminal {
public:
    Terminal(u64 arg);
    void run();

private:
    bool init();
    void process_key(middlespace::Key key);
    ustd::string get_line();
    middlespace::Key get_key();
    void suggest_cmd(const ustd::string& cmd);
    void suggest_param(const ustd::string& param);

    void process_cmd(const ustd::string& cmd);
    void print_klog();

    template <class T>
    void install_cmd(const ustd::string& cmd) {
        cmds::CmdBase* task = new T((u64)&env);
        cmd_collection.install(cmd, task);
    }

    std::tuple<bool, ustd::string, ustd::string> cd_filter(const ustd::string& pattern);
    static const ustd::string PROMPT;

    ustd::string cl_user_input;
    middlespace::Key last_key = middlespace::Key::INVALID;
    CommandHistory cmd_history;
    CommandCollection cmd_collection;


    utils::ScrollableScreenPrinter* printer;
    TerminalEnv env;

    int keyboard    = -1;
    int stdout      = -1;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINAL_H_ */
