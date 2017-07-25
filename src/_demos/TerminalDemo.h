/*
 * TerminalDemo.h
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_TERMINALDEMO_H_
#define SRC__DEMOS_TERMINALDEMO_H_

#include "ScreenPrinter.h"
#include "KeyboardDriver.h"
#include "kstd.h"

namespace demos {

class TerminalDemo {
public:
    void run();

private:
    bool init();
    void on_key_press(drivers::Key key);
    void process_key(drivers::Key key, kstd::string& line);
    kstd::string get_line();
    drivers::Key get_key();
    void suggest_cmd(const kstd::string& cmd);

    void process_cmd(const kstd::string& cmd);
    void print_klog();

    kstd::string line;
    utils::ScrollableScreenPrinter printer;
    drivers::Key last_key = drivers::Key::INVALID;
    kstd::vector<kstd::string> cmd_history;
    u16 cmd_history_index {0};
};

} /* namespace demos */

#endif /* SRC__DEMOS_TERMINALDEMO_H_ */
