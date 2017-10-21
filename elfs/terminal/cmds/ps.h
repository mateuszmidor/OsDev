/*
 * ps.h
 *
 *  Created on: Jul 27, 2017
 *      Author: mateusz
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_PS_H_
#define SRC_UTILS_TERMINAL_CMDS_PS_H_

#include "CmdBase.h"

namespace cmds {

class ps: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_PS_H_ */
