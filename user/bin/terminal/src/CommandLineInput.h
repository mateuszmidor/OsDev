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
    CommandLineInput(ustd::Monitor<ScrollableScreenPrinter>& printer);

    void prompt();
    void backspace();
    void putc(char c);
    void clear();

    void install(const ustd::string& cmd_name);
    const ustd::string& get_text() const { return input; }
    void suggest_cmd(const ustd::string& cmd);
    void suggest_param(const ustd::string& param);
    void help_me_out();

    ustd::Monitor<ScrollableScreenPrinter>&    printer;

private:
    std::tuple<bool, ustd::string> command_filter(const ustd::string& pattern);
    std::tuple<bool, ustd::string, ustd::string> folder_filter(const ustd::string& cwd, const ustd::string& pattern);

    static const char           PROMPT[];
    static char                 cwd[];
    ustd::vector<ustd::string>  known_commands;
    ustd::string                input;
};

} /* namespace terminal */

#endif /* ELFS_TERMINAL_COMMANDLINEINPUT_H_ */
