/**
 *   @file: elfrun.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "elfrun.h"
#include "kstd.h"
#include "Elf64.h"
#include "TaskManager.h"
#include "PageTables.h"
#include "MemoryManager.h"

using namespace kstd;
using namespace utils;
using namespace memory;
using namespace filesystem;
using namespace multitasking;

namespace cmds {

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
 * ELF image is organized as follows:
 * 0...x -> process image
 * ELF_VIRTUAL_MEM_BYTES - stack size -> process stack
 */
void elfrun::run() {
    if (env->volumes.empty()) {
        env->printer->format("elfrun: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("elfrun: please specify file name\n");
        return;
    }

    string name = env->cmd_args[1];
    string filename = env->cwd + "/" + name;
    VolumeFat32* v = env->volume;
    auto e = v->get_entry(filename);
    if (!e) {
        env->printer->format("elfrun: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory()) {
        env->printer->format("elfrun: % is a directory\n", filename);
        return;
    }


    MemoryManager& mm = MemoryManager::instance();
    size_t pml4_phys_addr = (size_t)mm.alloc_frames(sizeof(hardware::PageTables64)); // must be physical, continuous address space
    if (!pml4_phys_addr) {
        env->printer->format("elfrun: not enough memory to run elf\n");
        return;
    }

    // create elf address space mapping, elf_loader will use this address space and load elf segments into it
    hardware::PageTables::map_elf_address_space(pml4_phys_addr);

    // prepare elf run environment
    RunElfParams* run_env = new RunElfParams;
    run_env->pml4_phys_addr = pml4_phys_addr;
    run_env->argc = env->cmd_args.size() - 1; // no "elfrun" command
    run_env->argv = string_vec_to_argv(env->cmd_args); // should take userspace allocator

    // read elf file data
    u32 size = e.get_size();
    run_env->elf_file_data = new u8[size];
    e.read(run_env->elf_file_data, size);

    // run task loaded in task target address space
    Task task(load_and_run_elf, "elf_loader", (u64)run_env, false, pml4_phys_addr);
    TaskManager& task_manager = TaskManager::instance();
    u32 tid = task_manager.add_task(task);
}

/**
 * @brief   ELF loader kernel task that runs already in the task address space
 *          and thus is able to load the elf program segments into memory at virtual addresses starting at "0"
 *
 */
void elfrun::load_and_run_elf(u64 arg) {
    RunElfParams* run_env = (RunElfParams*)arg;

    // copy file sections into address space
    run_env->entry_point = Elf64::load_into_current_addressspace(run_env->elf_file_data);
    delete run_env->elf_file_data; // data copied and no longer needed

    // prepare elf task stack
    const u64 ELF_STACK_SIZE = 4 * 4096;
    const u64 ELF_STACK =  ELF_VIRTUAL_MEM_BYTES - ELF_STACK_SIZE; // use top of the elf virtual memory for the stack

    Task task(run_elf_in_current_addressspace, run_env->argv[0], arg, true, run_env->pml4_phys_addr, ELF_STACK, ELF_STACK_SIZE);
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
[[noreturn]] void elfrun::run_elf_in_current_addressspace(u64 arg) {
    RunElfParams* params = (RunElfParams*)arg;

    asm volatile(
            "jmp *%0\n;"   // jump to "main" function address
            :
            : "g"(params->entry_point), "D"(params->argc), "S"(params->argv)
    );

    __builtin_unreachable();
}

/**
 * @brief   Convert vector<string> into char*[]
 */
char** elfrun::string_vec_to_argv(const vector<string>& src_vec) {
    u8 argc = src_vec.size() - 1;   // no first ("elfrun" cmd) item
    char** argv = new char* [argc];
    for (u8 i = 0; i < argc; i++) {
        const string& src = src_vec[i + 1];
        argv[i] = new char[src.length() + 1]; // +1 for null terminator
        memcpy(argv[i], src.c_str(), src.length() + 1); // +1 for null terminator
    }

    return argv;
}
} /* namespace cmds */
