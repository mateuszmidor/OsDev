/**
 *   @file: CmdBase.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_
#define SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_

#include "TerminalEnv.h"

namespace cmds {

class CmdBase {
public:
    CmdBase(u64 arg);
    virtual ~CmdBase() {}
    virtual void run() = 0;
    kstd::string make_absolute_filename(const kstd::string& relative_filename) const;

protected:
    terminal::TerminalEnv* env;

};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CMDBASE_H_ */
