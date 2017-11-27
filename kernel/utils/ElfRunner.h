/**
 *   @file: ElfRunner.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_USERSPACE_ELFRUNNER_H_
#define SRC_USERSPACE_ELFRUNNER_H_

#include "kstd.h"
#include "Vector.h"
#include "BumpAllocationPolicy.h"

namespace utils {

/**
 * @brief   This class runs elfs from kernel address space
 */
class ElfRunner {
public:
    s32 run(u8* elf_data, kstd::vector<kstd::string>* argv) const;

private:
    static void load_and_run_elf(u8* elf_file_data, kstd::vector<kstd::string>* args);
    static char** string_vec_to_argv(const kstd::vector<kstd::string>& src_vec, memory::BumpAllocationPolicy& allocator);

    static const size_t ELF_VIRTUAL_MEM_BYTES   = 1024*1024*1024;  // 1GB of virtual memory can be dynamically mapped on Page Fault, as for now
    static const size_t ELF_STACK_SIZE          = 4 * 4096;
    static const size_t ELF_STACK_START         = ELF_VIRTUAL_MEM_BYTES - ELF_STACK_SIZE; // use top of the elf virtual memory for the stack
};

} /* namespace userspace */

#endif /* SRC_USERSPACE_ELFRUNNER_H_ */
