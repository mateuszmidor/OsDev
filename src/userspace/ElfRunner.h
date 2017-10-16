/**
 *   @file: ElfRunner.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_USERSPACE_ELFRUNNER_H_
#define SRC_USERSPACE_ELFRUNNER_H_

#include "kstd.h"

namespace userspace {

/**
 * @brief   Because multitasking::Task only takes 1 u64 argument, we pack run environment in a struct and pass it to the Task
 */
struct RunElfParams {
    RunElfParams() : entry_point(0), argc(0), argv(0), pml4_phys_addr(0), elf_file_data(0) {}
    u8* elf_file_data;
    u64 pml4_phys_addr;
    u64 entry_point;
    u64 argc;
    char** argv;
};

/**
 * @brief   This class runs elfs from kernel address space
 */
class ElfRunner {
public:
    bool run(u8* elf_data, const kstd::vector<kstd::string> argv) const;

private:
    static void load_and_run_elf(u64 arg);
    [[noreturn]] static void run_elf_in_current_addressspace(u64 arg);
    char** string_vec_to_argv(const kstd::vector<kstd::string>& src_vec) const;

    static const size_t ELF_VIRTUAL_MEM_BYTES = 1024*1024*1024;  // 1GB of virtual memory can be mapped as for now
};

} /* namespace userspace */

#endif /* SRC_USERSPACE_ELFRUNNER_H_ */
