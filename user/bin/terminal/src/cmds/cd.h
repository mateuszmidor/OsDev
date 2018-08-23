/**
 *   @file: cd.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CD_H_
#define SRC_UTILS_TERMINAL_CMDS_CD_H_

#include "CmdBase.h"

namespace cmds {

class cd : public CmdBase {
public:
    using CmdBase::CmdBase;
    u32 run(const CmdArgs& args);

private:
    void navigate_path(const cstd::string& path);
    void cd_root();
    void navigate_back();
    void cd_directory(const cstd::string& path);
    cstd::string normalize_path(const cstd::string& path) const;
    void store_last_location();
    cstd::string get_cwd() const;

    // for navigating "back". Maybe should be stored in TerminalEnv?
    static cstd::string prev_cwd;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CD_H_ */
