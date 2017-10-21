/**
 *   @file: testfat32.h
 *
 *   @date: Aug 9, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_TESTFAT32_H_
#define SRC_UTILS_TERMINAL_CMDS_TESTFAT32_H_

#include "CmdBase.h"

namespace cmds {

class test_fat32: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
    void cleanup();
    void generate();
    void remove();

    static const u32 NUM_ENTRIES = 256;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_TESTFAT32_H_ */
