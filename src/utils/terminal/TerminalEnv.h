/**
 *   @file: TerminalEnv.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_TERMINALENV_H_
#define SRC_UTILS_TERMINAL_TERMINALENV_H_

#include "kstd.h"
#include "KernelLog.h"
#include "ScrollableScreenPrinter.h"
#include "VfsManager.h"

namespace terminal {

struct TerminalEnv {
    TerminalEnv();
    void install_volumes(drivers::AtaDevice& hdd);

    filesystem::VfsManager&     vfs_manager;
    utils::KernelLog* klog;
    utils::ScrollableScreenPrinter* printer;
    kstd::string cwd;

    kstd::vector<kstd::string> cmd_args;    // command_name, arg1, arg2, ...,  argn
};

} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_TERMINALENV_H_ */
