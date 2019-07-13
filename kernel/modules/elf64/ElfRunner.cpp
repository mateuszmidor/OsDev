/**
 *   @file: ElfRunner.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "Elf64.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "MemoryManager.h"
#include "ElfRunner.h"
#include "AddressSpaceManager.h"

using namespace cstd;
using namespace utils;
using namespace memory;
using namespace middlespace;
using namespace multitasking;

namespace elf64 {

/**
 * @brief   Convert vector<string> into char*[] that is stored in user space virtual memory
 */
static char** string_vec_to_argv(const vector<string>& src_vec, AddressSpace& address_space) {
    u8 argc = src_vec.size();
    char** argv =  (char**)memory::alloc_static(address_space, argc * sizeof(char*));
    for (u8 i = 0; i < argc; i++) {
        const string& src = src_vec[i];
        argv[i] = (char*)memory::alloc_static(address_space, src.length() + 1); // +1 for null terminator
        memcpy(argv[i], src.c_str(), src.length() + 1); // +1 for null terminator
    }

    return argv;
}

/**
 * @brief   ELF loader kernel task that runs already in the task address space
 *          and thus is able to load the elf program segments into memory at virtual addresses starting at "0"
 * @note    This function frees the elf_file_data and args
 *          The actual task gets task_id of elf_loader
 */
static void load_and_run_elf(u8* elf_file_data, vector<string>* args) {
    // copy elf sections into address space and remember where free virtual memory starts
    size_t entry_point = utils::Elf64::load_into_current_addressspace(elf_file_data);
    delete[] elf_file_data;

    TaskManager& task_manager = TaskManager::instance();
    Task& elf_loader_task =  task_manager.get_current_task();

    // prepare the target run environment in use memory space
    size_t argc = args->size();
    char** argv = string_vec_to_argv(*args, elf_loader_task.task_group_data->address_space);
    delete args;

    // run the actual elf task
    Task* task = TaskFactory::make_lightweight_task(elf_loader_task, entry_point, argv[0], Task::DEFAULT_USER_STACK_SIZE)->set_arg1(argc)->set_arg2(argv); // reuse prepared address space
    task->is_user_space = true;                                                                                                             // but make the elf task a userspace task

    // replace elf_loader with actual ELF task
    task_manager.replace_current_task(task);
}

/**
 * @brief   Prepare a task for loading elf program.
 *          This task runs in kernel space, but owns its own address space 'as' to load the elf code&data into
 */
static Task* make_elf_loader_task(const AddressSpace& as) {
    TaskManager& task_manager = TaskManager::instance();
    const auto& current = task_manager.get_current_task();
    const string& CWD = current.task_group_data->cwd;           // inherit current working directory

    Task* task = TaskFactory::make_kernel_task(load_and_run_elf, "elf_loader"); // kernel task so it can run "load_and_run_elf"
    task->task_group_data  = cstd::make_shared<TaskGroupData>(as, CWD, current.task_id); // but in its own address space
    return task;
}

/**
 * @brief   Run elf as user task and return immediately
 * @param   elf_data ELF64 file data loaded into kernel memory. This function finally free the "elf_data"
 * @param   args User-provided cmd line arguments
 * @return  Error code on error, task id on success
 */
SyscallResult<u32> ElfRunner::run(u8* elf_data, const vector<string>& args) const {
    // check if elf_data points to actual Elf64
    if (!Elf64::is_elf64(elf_data))
        return {ErrorCode::EC_NOEXEC};

    // try alloc new address space for our elf program
    AddressSpace as;
    if (auto result = alloc_address_space(Elf64::get_available_memory_first_byte(elf_data), ELF_VIRTUAL_MEM_BYTES))
        as = result.value;
    else
        return {result.ec};

    // alloc arguments in kernel address space so they can be safely passed to elf_loader kernel task
    vector<string>* pargs = new vector<string>(args);

    // prepare elf_loader kernel task with fresh address space to load elf segments into
    Task* elf_loader = make_elf_loader_task(as)->set_arg1(elf_data)->set_arg2(pargs);

    // try run the elf_loader; it will release the memory
    TaskManager& task_manager = TaskManager::instance();
    if (u32 tid = task_manager.add_task(elf_loader)) {
        return {tid};
    }
    // on failure release the memory by ourself
    else {
        delete[] elf_data;
        delete pargs;
        return {ErrorCode::EC_PERM};  // running new task not permitted at this time
    }
}

} /* namespace elf64 */
