/**
 *   @file: Terminal.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINAL_H_
#define SRC_UTILS_TERMINAL_TERMINAL_H_

#include "Monitor.h"
#include "Keyboard.h"
#include "CommandHistory.h"
#include "CommandCollection.h"
#include "CommandLineInput.h"
#include "ScrollableScreenPrinter.h"

namespace terminal {

class Terminal {
public:
    Terminal(const cstd::string& terminal_binary_name);
    void run();

private:
    bool init();
    void setup_screen_and_print_welcome();
    void install_internal_command(cmds::CmdBase* cmd, const cstd::string& cmd_name);
    void install_external_commands(const cstd::string& dir, const cstd::string& omit_name);
    void on_key_down(middlespace::Key key);
    void process_cmd(const cstd::string& cmd);
    bool run_cmd(const cstd::vector<cstd::string>& cmd_args, bool run_in_bg);

    static void stdout_printer_thread(Terminal* term);
    static void key_processor_thread(Terminal* term);

    const cstd::string  self_binary_name;           // "TERMINAL"
    CommandHistory      cmd_history;
    CommandCollection   cmd_collection;
    CommandLineInput    user_input;

    cstd::ustd::Monitor<ScrollableScreenPrinter> printer; // printer is accessed from multiple threads so it is secured inside Monitor

    // file descriptors
    int fd_keyboard     {-1};   // for reading keyboard input
    int fd_stdin        {-1};   // for reading formatted string user input
    int fd_stdout       {-1};   // for reading stdout of child tasks(elf64)
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINAL_H_ */
