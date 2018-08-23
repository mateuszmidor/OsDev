/**
 *   @file: specificelfrun.h
 *
 *   @date: Oct 24, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_TERMINAL_CMDS_SPECIFICELFRUN_H_
#define ELFS_TERMINAL_CMDS_SPECIFICELFRUN_H_

#include "CmdBase.h"

namespace cmds {

class specificelfrun: public CmdBase {
public:
    specificelfrun(const cstd::string& elf_absolute_path);
    ~specificelfrun() override {}
    u32 run(const CmdArgs& args) override;

private:
    cstd::string elf_path;
};

} /* namespace cmds */

#endif /* ELFS_TERMINAL_CMDS_SPECIFICELFRUN_H_ */
