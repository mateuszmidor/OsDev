/**
 *   @file: Terminal.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINAL_H_
#define SRC_UTILS_TERMINAL_TERMINAL_H_

#include "Keyboard.h"
#include "CommandHistory.h"
#include "CommandCollection.h"
#include "CommandLineInput.h"
#include "Monitor.h"
#include "ustd.h"

namespace terminal {

class Terminal {
public:
    Terminal(u64 arg);
    void run();

private:
    bool init();
    void on_key_down(middlespace::Key key);
    void process_cmd(const ustd::string& cmd);

    void install_external_commands(const ustd::string& dir);
    void install_cmd(cmds::CmdBase* cmd, const ustd::string& cmd_name);

    static void stdout_printer_thread(Terminal* term);
    static void key_processor_thread(Terminal* term);

    CommandHistory      cmd_history;
    CommandCollection   cmd_collection;
    CommandLineInput    user_input;

    ustd::Monitor<ScrollableScreenPrinter> printer; // printer is accessed from multiple threads so it is secured inside Monitor

    // file descriptors
    int fd_keyboard     = -1;   // for reading keyboard input
    int fd_stdin        = -1;   // for reading formatted string user input
    int fd_stdout       = -1;   // for reading stdout of child tasks(elf64)
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINAL_H_ */
