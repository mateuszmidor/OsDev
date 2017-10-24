/**
 *   @file: TerminalEnv.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINALENV_H_
#define SRC_UTILS_TERMINAL_TERMINALENV_H_

#include "ustd.h"
#include "ScrollableScreenPrinter.h"

namespace terminal {

struct TerminalEnv {
    TerminalEnv();

    ScrollableScreenPrinter* printer;
    ustd::string cwd;

    ustd::vector<ustd::string> cmd_args;    // command_name, arg1, arg2, ...,  argn
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINALENV_H_ */
