/**
 *   @file: ElfRunner.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "Elf64.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageTables.h"
#include "MemoryManager.h"
#include "ElfRunner.h"

using namespace kstd;
using namespace memory;
using namespace multitasking;
namespace utils {

/**
 * @brief   Run elf as user task and return immediately
 * @param   elf_data ELF64 file data loaded into kernel memory. This function finally free the "elf_data"
 * @param   args User-provided cmd line arguments in kernel memory. This function finally free the "args"
 * @return  Error code on error, task id on success
 */
s32 ElfRunner::run(u8* elf_data, kstd::vector<kstd::string>* args) const {
    if (!utils::Elf64::is_elf64(elf_data))
        return -ENOEXEC; // not executable

    MemoryManager& mm = MemoryManager::instance();
    size_t pml4_phys_addr = (size_t)mm.alloc_frames(sizeof(PageTables64)); // must be physical, continuous address space
    if (!pml4_phys_addr)
        return -ENOMEM;

    // create elf address space mapping, elf_loader will use this address space and load elf segments into it
    PageTables::map_elf_address_space(pml4_phys_addr);

    // run elf_loader in target address space, so the elf segments can be loaded
    TaskManager& task_manager = TaskManager::instance();
    Task* task = TaskFactory::make_kernel_task(load_and_run_elf, "elf_loader")->set_arg1(elf_data)->set_arg2(args);
    task->pml4_phys_addr = pml4_phys_addr;   // kernel task but in user memory space
    task->cwd = task_manager.get_current_task().cwd; // inherit current working directory of calling task

    if (u32 tid = task_manager.add_task(task)) {
        return tid;
    }
    else
        return -EPERM;  // running new task not permitted at this time
}

/**
 * @brief   ELF loader kernel task that runs already in the task address space
 *          and thus is able to load the elf program segments into memory at virtual addresses starting at "0"
 * @note    This function frees the elf_file_data and args
 *          The actual task gets task_id of elf_loader
 */
void ElfRunner::load_and_run_elf(u8* elf_file_data, vector<string>* args) {
    // copy elf sections into address space and remember where free virtual memory starts
    u64 entry_point = utils::Elf64::load_into_current_addressspace(elf_file_data);
    u64 free_virtual_mem_start = utils::Elf64::get_available_memory_first_byte(elf_file_data);
    delete[] elf_file_data;

    // free to use user space memory is between elf image last byte and elf stack first byte
    BumpAllocationPolicy userspace_allocator(free_virtual_mem_start, ELF_VIRTUAL_MEM_BYTES - ELF_STACK_SIZE);

    // prepare the target run environment in use memory space
    u64 argc = args->size();
    char** argv = string_vec_to_argv(*args, userspace_allocator);
    delete args;

    // run the actual elf task
    TaskManager& task_manager = TaskManager::instance();
    Task& elf_loader_task =  task_manager.get_current_task();
    Task* task = TaskFactory::make_user_task(entry_point, argv[0], elf_loader_task.pml4_phys_addr, ELF_STACK_START, ELF_STACK_SIZE)->set_arg1(argc)->set_arg2(argv);
    task->cwd = elf_loader_task.cwd; // inherit current working directory of calling task

    // replace elf_loader with actual ELF task
    task_manager.replace_current_task(task);
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
