/**
 *   @file: lspci.h
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_LSPCI_H_
#define SRC_UTILS_TERMINAL_CMDS_LSPCI_H_

#include "CmdBase.h"

namespace cmds {

class lspci: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_LSPCI_H_ */
