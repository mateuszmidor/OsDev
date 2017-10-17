/**
 *   @file: cd.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_CD_H_
#define SRC_UTILS_TERMINAL_CMDS_CD_H_

#include "CmdBase.h"
#include "AtaDriver.h"
#include "TerminalEnv.h"

namespace cmds {

class cd : public CmdBase {
public:
    using CmdBase::CmdBase;
    void run();

private:
    void navigate_path(const kstd::string& path);
    void cd_root();
    void navigate_back();
    void cd_volume_directory(const kstd::string& path);
    void split_volume_path(const kstd::string& location, kstd::string& volume, kstd::string& path) const;
    void cd_directory(const kstd::string& path);
    kstd::string normalize_path(const kstd::string& path) const;
    void store_last_location();

    // for navigating "back". Maybe should be stored in TerminalEnv?
    static filesystem::VolumeFat32* prev_volume;
    static kstd::string prev_cwd;
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_CD_H_ */
