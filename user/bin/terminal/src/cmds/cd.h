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
    void run(const CmdArgs& args,bool run_in_bg = false);

private:
    void navigate_path(const ustd::string& path);
    void cd_root();
    void navigate_back();
    void cd_directory(const ustd::string& path);
    ustd::string normalize_path(const ustd::string& path) const;
    void store_last_location();
    ustd::string get_cwd() const;

    // for navigating "back". Maybe should be stored in TerminalEnv?
    static ustd::string prev_cwd;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CD_H_ */
