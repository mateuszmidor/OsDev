/**
 *   @file: ElfRunner.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_USERSPACE_ELFRUNNER_H_
#define SRC_USERSPACE_ELFRUNNER_H_

#include "Vector.h"
#include "SyscallResult.h"

namespace utils {

/**
 * @brief   This class runs elfs from kernel address space
 */
class ElfRunner {
public:
    utils::SyscallResult<u32> run(u8* elf_data, const cstd::vector<cstd::string>& args) const;

private:
    static void load_and_run_elf(u8* elf_file_data, cstd::vector<cstd::string>* args);
    static constexpr size_t ELF_VIRTUAL_MEM_BYTES   = 1024*1024*1024;  // 1GB of virtual memory can be dynamically mapped on Page Fault, as for now
};

} /* namespace userspace */

#endif /* SRC_USERSPACE_ELFRUNNER_H_ */
