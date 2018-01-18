/**
 *   @file: CommandLineInput.h
 *
 *   @date: Oct 24, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_TERMINAL_COMMANDLINEINPUT_H_
#define ELFS_TERMINAL_COMMANDLINEINPUT_H_

#include <tuple>
#include "Monitor.h"
#include "ScrollableScreenPrinter.h"

namespace terminal {

class CommandLineInput {
public:
    CommandLineInput(cstd::ustd::Monitor<ScrollableScreenPrinter>& printer);

    void prompt();
    void backspace();
    void putc(char c);
    void clear();

    void install(const cstd::string& cmd_name);
    const cstd::string& get_text() const { return input; }
    void suggest_cmd(const cstd::string& cmd);
    void suggest_param(const cstd::string& param);
    void help_me_out();

    cstd::ustd::Monitor<ScrollableScreenPrinter>&    printer;

private:
    std::tuple<bool, cstd::string> command_filter(const cstd::string& pattern);
    std::tuple<bool, cstd::string, cstd::string> folder_filter(const cstd::string& cwd, const cstd::string& pattern);

    static const char           PROMPT[];
    static char                 cwd[];
    cstd::vector<cstd::string>  known_commands;
    cstd::string                input;
};

} /* namespace terminal */

#endif /* ELFS_TERMINAL_COMMANDLINEINPUT_H_ */
