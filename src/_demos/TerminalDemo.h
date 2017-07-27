/*
 * TerminalDemo.h
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_TERMINALDEMO_H_
#define SRC__DEMOS_TERMINALDEMO_H_

#include "ScrollableScreenPrinter.h"
#include "KeyboardDriver.h"
#include "Task.h"
#include <memory>
#include <tuple>
#include "kstd.h"

namespace demos {

class CommandHistory {
public:
    CommandHistory();
    void append(const kstd::string& cmd);
    void set_to_latest();
    const kstd::string& get_prev();
    const kstd::string& get_next();

private:
    kstd::vector<kstd::string> history;
    u16 index   {0};
};

struct Command {
    kstd::string            name;
    multitasking::TaskPtr   task;
};

class CommandCollection {
public:
    CommandCollection();
    multitasking::TaskPtr get(const kstd::string& cmd_name);
    std::tuple<bool, kstd::string> filter(const kstd::string& pattern);
    void install(const kstd::string name, multitasking::TaskPtr task);

private:
    kstd::vector<Command> commands;
};

class TerminalDemo {
public:
    TerminalDemo();
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

} /* namespace demos */

#endif /* SRC__DEMOS_TERMINALDEMO_H_ */
