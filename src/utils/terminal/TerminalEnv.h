/**
 *   @file: TerminalEnv.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINALENV_H_
#define SRC_UTILS_TERMINAL_TERMINALENV_H_

#include "VolumeFat32.h"
#include "kstd.h"
#include "AtaDriver.h"
#include "KernelLog.h"
#include "ScrollableScreenPrinter.h"

namespace terminal {

struct TerminalEnv {
    TerminalEnv();
    void install_volumes(drivers::AtaDevice& hdd);

    utils::KernelLog* klog;
    utils::ScrollableScreenPrinter* printer;
    filesystem::VolumeFat32* volume;
    kstd::string cwd;
    kstd::vector<filesystem::VolumeFat32> volumes;


    kstd::vector<kstd::string> cmd_args;    // command_name, arg1, arg2, ...,  argn
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINALENV_H_ */
