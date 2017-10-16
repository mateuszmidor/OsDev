/**
 *   @file: Terminal.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINAL_H_
#define SRC_UTILS_TERMINAL_TERMINAL_H_


#include "TerminalEnv.h"
#include "KeyboardDriver.h"
#include "CommandHistory.h"
#include "CommandCollection.h"
#include "TaskFactory.h"
#include "kstd.h"
#include <tuple>

namespace terminal {

class Terminal {
public:
    Terminal(u64 arg);
    void run();

private:
    bool init();
    void on_key_press(drivers::Key key);
    void process_key(drivers::Key key);
    kstd::string get_line();
    drivers::Key get_key();
    void suggest_cmd(const kstd::string& cmd);

    void process_cmd(const kstd::string& cmd);
    void print_klog();

    template <class T>
    void install_cmd(const kstd::string& cmd) {
        cmd_collection.install(cmd, multitasking::TaskFactory::make_kernel_task<T>(cmd, (u64)&env));
    }

    static const kstd::string PROMPT;

    kstd::string edit_line;
    drivers::Key last_key = drivers::Key::INVALID;
    CommandHistory cmd_history;
    CommandCollection cmd_collection;

    utils::KernelLog& klog;
    utils::ScrollableScreenPrinter printer;
    TerminalEnv env;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINAL_H_ */
