/**
 *   @file: cat.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CAT_H_
#define SRC_UTILS_TERMINAL_CMDS_CAT_H_

#include "CmdBase.h"

namespace cmds {

class cat: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CAT_H_ */
