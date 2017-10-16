/**
 *   @file: ElfRunner.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_USERSPACE_ELFRUNNER_H_
#define SRC_USERSPACE_ELFRUNNER_H_

#include "kstd.h"
#include "BumpAllocationPolicy.h"

namespace userspace {

/**
 * @brief   Because multitasking::Task only takes 1 u64 argument, we pack run environment in a struct and pass it to the Task
 */
struct RunElfParams {
    RunElfParams() : pml4_phys_addr(0), elf_file_data(0), args(0) {}
    const kstd::vector<kstd::string>* args;     // cmd line args, in kernel space
    u8* elf_file_data;                          // elf file data; allocated in kernel space
    u64 pml4_phys_addr;                         // elf task page tables phys address
};

/**
 * @brief   This class runs elfs from kernel address space
 */
class ElfRunner {
public:
    bool run(u8* elf_data, const kstd::vector<kstd::string>& argv) const;

private:
    static void load_and_run_elf(u64 arg);
    static char** string_vec_to_argv(const kstd::vector<kstd::string>& src_vec, memory::BumpAllocationPolicy& allocator);

    static const size_t ELF_VIRTUAL_MEM_BYTES   = 1024*1024*1024;  // 1GB of virtual memory can be dynamically mapped on Page Fault, as for now
    static const size_t ELF_STACK_SIZE          = 4 * 4096;
    static const size_t ELF_STACK_START         = ELF_VIRTUAL_MEM_BYTES - ELF_STACK_SIZE; // use top of the elf virtual memory for the stack
};

} /* namespace userspace */

#endif /* SRC_USERSPACE_ELFRUNNER_H_ */
