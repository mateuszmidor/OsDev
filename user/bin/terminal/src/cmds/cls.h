/**
 *   @file: cls.h
 *
 *   @date: Jan 9, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_BIN_TERMINAL_SRC_CMDS_CLS_H_
#define USER_BIN_TERMINAL_SRC_CMDS_CLS_H_

#include "CmdBase.h"
#include "Monitor.h"
#include "ScrollableScreenPrinter.h"

namespace cmds {

class cls: public CmdBase {
public:
    cls(ustd::Monitor<terminal::ScrollableScreenPrinter>& printer) : printer(printer) {}
    void run(const CmdArgs& args,bool run_in_bg = false);

private:
    ustd::Monitor<terminal::ScrollableScreenPrinter>&    printer;
};

} /* namespace cmds */

#endif /* USER_BIN_TERMINAL_SRC_CMDS_CLS_H_ */
