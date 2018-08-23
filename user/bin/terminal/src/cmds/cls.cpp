/**
 *   @file: cls.cpp
 *
 *   @date: Jan 9, 2018
 * @author: Mateusz Midor
 */

#include "cls.h"

namespace cmds {

u32 cls::run(const CmdArgs& args) {
    auto handle = printer.get();
    handle->clear_screen();
    return 0;
}

} /* namespace cmds */
