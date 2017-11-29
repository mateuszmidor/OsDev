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
    if (!Elf64::is_elf64(elf_data))
        return -ENOEXEC; // not executable

    MemoryManager& mm = MemoryManager::instance();
    size_t pml4_phys_addr = (size_t)mm.alloc_frames(sizeof(PageTables64)); // must be physical, continuous address space
    if (!pml4_phys_addr)
        return -ENOMEM;

    // create elf address space mapping, elf_loader will use this address space and load elf segments into it
    PageTables::map_elf_address_space(pml4_phys_addr);

    // run load_and_run_elf in target address space, so the elf segments can be loaded
    TaskManager& task_manager = TaskManager::instance();

    const size_t HEAP_LOW_LIMIT = Elf64::get_available_memory_first_byte(elf_data);     // first free byte after ELF segments
    const size_t HEAP_HIGH_LIMIT = ELF_VIRTUAL_MEM_BYTES;
    const string& CWD = task_manager.get_current_task().task_group_data->cwd;           // inherit current working directory
    Task* task = TaskFactory::make_kernel_task(load_and_run_elf, "elf_loader")->set_arg1(elf_data)->set_arg2(args); // kernel task so it can run "load_and_run_elf"
    task->task_group_data  = std::make_shared<TaskGroupData>(pml4_phys_addr, CWD, HEAP_LOW_LIMIT, HEAP_HIGH_LIMIT); // but in its own address space

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
    size_t entry_point = Elf64::load_into_current_addressspace(elf_file_data);
    delete[] elf_file_data;

    TaskManager& task_manager = TaskManager::instance();
    Task& elf_loader_task =  task_manager.get_current_task();

    // prepare the target run environment in use memory space
    size_t argc = args->size();
    char** argv = string_vec_to_argv(*args, elf_loader_task.task_group_data);
    delete args;

    // run the actual elf task
    Task* task = TaskFactory::make_lightweight_task(elf_loader_task, entry_point, argv[0], ELF_STACK_SIZE)->set_arg1(argc)->set_arg2(argv); // reuse prepared address space
    task->is_user_space = true;                                                                                                             // but make the elf task a userspace task

    // replace elf_loader with actual ELF task
    task_manager.replace_current_task(task);
}

/**
 * @brief   Convert vector<string> into char*[] that is stored in user space virtual memory
 */
char** ElfRunner::string_vec_to_argv(const vector<string>& src_vec, TaskGroupDataPtr tgr) {
    u8 argc = src_vec.size();
    char** argv =  (char**)tgr->alloc_static(argc * sizeof(char*));
    for (u8 i = 0; i < argc; i++) {
        const string& src = src_vec[i];
        argv[i] = (char*)tgr->alloc_static(src.length() + 1); // +1 for null terminator
        memcpy(argv[i], src.c_str(), src.length() + 1); // +1 for null terminator
    }

    return argv;
}
} /* namespace userspace */
