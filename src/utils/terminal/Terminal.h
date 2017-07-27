/**
 *   @file: Terminal.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINAL_H_
#define SRC_UTILS_TERMINAL_TERMINAL_H_

#include "ScrollableScreenPrinter.h"
#include "KeyboardDriver.h"
#include "CommandHistory.h"
#include "CommandCollection.h"
#include "kstd.h"
#include <memory>
#include <tuple>

namespace terminal {

class Terminal {
public:
    Terminal();
    void run(u64 arg);

private:
    bool init();
    void on_key_press(drivers::Key key);
    void process_key(drivers::Key key);
    kstd::string get_line();
    drivers::Key get_key();
    void suggest_cmd(const kstd::string& cmd);

    void process_cmd(const kstd::string& cmd);
    void print_klog();

    static const kstd::string PROMPT;

    kstd::string edit_line;
    utils::ScrollableScreenPrinter printer;
    drivers::Key last_key = drivers::Key::INVALID;
    CommandHistory cmd_history;
    CommandCollection cmd_collection;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINAL_H_ */
