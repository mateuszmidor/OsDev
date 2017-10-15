/**
 *   @file: elfrun.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_
#define SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_

#include "kstd.h"
#include "CmdBase.h"

namespace cmds {

class elfrun: public CmdBase {
public:
    using CmdBase::CmdBase;
    void run() override;

private:
    static void load_and_run_elf(u64 arg);
    [[noreturn]] static void run_elf_in_current_addressspace(u64 arg);
    char** string_vec_to_argv(const kstd::vector<kstd::string>& src_vec);

    static const size_t ELF_VIRTUAL_MEM_BYTES = 1024*1024*1024;  // 1GB of virtual memory can be mapped as for now
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_ELFRUN_H_ */
