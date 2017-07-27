/**
 *   @file: TerminalEnv.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINALENV_H_
#define SRC_UTILS_TERMINAL_TERMINALENV_H_

#include "kstd.h"
#include "VolumeFat32.h"
#include "ScrollableScreenPrinter.h"

namespace terminal {

struct TerminalEnv {
    utils::ScrollableScreenPrinter* printer;
    filesystem::VolumeFat32* volume;
    kstd::string cwd;
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINALENV_H_ */
