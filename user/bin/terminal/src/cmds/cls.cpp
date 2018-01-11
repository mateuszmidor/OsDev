/**
 *   @file: cls.cpp
 *
 *   @date: Jan 9, 2018
 * @author: Mateusz Midor
 */

#include "cls.h"

namespace cmds {

void cls::run(const CmdArgs& args, bool run_in_bg) {
    auto handle = printer.get();
    handle->clear_screen();
}

} /* namespace cmds */
