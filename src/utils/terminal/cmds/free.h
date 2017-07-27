/*
 * free.h
 *
 *  Created on: Jul 27, 2017
 *      Author: mateusz
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_FREE_H_
#define SRC_UTILS_TERMINAL_CMDS_FREE_H_

#include "CmdBase.h"

namespace cmds {

class free: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_FREE_H_ */
