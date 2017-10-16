/**
 *   @file: ElfRunner.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "Elf64.h"
#include "TaskManager.h"
#include "PageTables.h"
#include "MemoryManager.h"
#include "ElfRunner.h"

using namespace kstd;
using namespace memory;
using namespace multitasking;
namespace userspace {

/**
 * @brief   Run elf as user task
 * @param   elf_data ELF64 file data loaded into kernel memory. This function doesnt free the data.
 * @param   args User-provided cmd line arguments in kernel memory
 * @return  True if elf_loader run successfuly, False otherwise
 */
bool ElfRunner::run(u8* elf_data, const kstd::vector<kstd::string>& args) const {
    MemoryManager& mm = MemoryManager::instance();
    size_t pml4_phys_addr = (size_t)mm.alloc_frames(sizeof(hardware::PageTables64)); // must be physical, continuous address space
    if (!pml4_phys_addr)
        return false;

    // create elf address space mapping, elf_loader will use this address space and load elf segments into it
    hardware::PageTables::map_elf_address_space(pml4_phys_addr);

    // prepare elf_loader run environment
    RunElfParams* run_env = new RunElfParams;
    run_env->pml4_phys_addr = pml4_phys_addr;
    run_env->elf_file_data = elf_data;
    run_env->args = &args;

    // run elf_loader in target address space, so the elf segments can be loaded
    Task task(load_and_run_elf, "elf_loader", (u64)run_env, false, pml4_phys_addr);
    TaskManager& task_manager = TaskManager::instance();
    if (u32 tid = task_manager.add_task(task)) {
        task_manager.wait(tid); // wait so that the elf_data nor args get freed too early
        return true;
    } else
        return false;
}

/**
 * @brief   ELF loader kernel task that runs already in the task address space
 *          and thus is able to load the elf program segments into memory at virtual addresses starting at "0"
 */
void ElfRunner::load_and_run_elf(u64 arg) {
    RunElfParams* run_env = (RunElfParams*)arg;

    // copy elf sections into address space and remember where free virtual memory starts
    run_env->entry_point = utils::Elf64::load_into_current_addressspace(run_env->elf_file_data);
    u64 free_virtual_mem_start = utils::Elf64::get_available_memory_first_byte(run_env->elf_file_data);
    delete run_env->elf_file_data; // data copied and no longer needed

    // free to use user space memory is between elf image last byte and elf stack first byte
    BumpAllocationPolicy userspace_allocator(free_virtual_mem_start, ELF_VIRTUAL_MEM_BYTES - ELF_STACK_SIZE);

    // prepare the target run environment
    run_env->argc = run_env->args->size();
    run_env->argv = string_vec_to_argv(*run_env->args, userspace_allocator);

    // run the actual elf task
    Task task(run_elf_in_current_addressspace, run_env->argv[0], arg, true, run_env->pml4_phys_addr, ELF_STACK_START, ELF_STACK_SIZE);
    TaskManager& task_manager = TaskManager::instance();
    task_manager.add_task(task);

    // page tables are shared by elf_loader and the actual user task to be run. make sure they are not wiped out upon elf_load erexit
    task_manager.get_current_task().pml4_phys_addr = 0;
}

/**
 * @brief   Prepare execution environment(argc and argv for "main" function) and jump to main function
 * @note    RunElfParams and all pointers in it will leak;
 *          in the future they will be allocated in the process address space and cleaned up with it when process is done
 * @note    This function never returns; instead int 0x80 for sys_exit is called from within the elf itself
 */
[[noreturn]] void ElfRunner::run_elf_in_current_addressspace(u64 arg) {
    RunElfParams* params = (RunElfParams*)arg;

    asm volatile(
            "jmp *%0\n;"   // jump to "main" function address
            :
            : "g"(params->entry_point), "D"(params->argc), "S"(params->argv)
    );

    __builtin_unreachable();
}

/**
 * @brief   Convert vector<string> into char*[] that is stored in user space virtual memory
 */
char** ElfRunner::string_vec_to_argv(const vector<string>& src_vec, BumpAllocationPolicy& allocator) {
    u8 argc = src_vec.size();
    char** argv =  (char**)allocator.alloc_bytes(argc * sizeof(char*));
    for (u8 i = 0; i < argc; i++) {
        const string& src = src_vec[i];
        argv[i] = (char*)allocator.alloc_bytes(src.length() + 1); // +1 for null terminator
        memcpy(argv[i], src.c_str(), src.length() + 1); // +1 for null terminator
    }

    return argv;
}
} /* namespace userspace */
