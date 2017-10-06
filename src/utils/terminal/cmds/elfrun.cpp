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
 * @brief   Because multitasking::Task only takes 1 u64 argument, we pack (entry point address, argc, argv) in a struct and pass it to the Task
 */
struct RunElfParams {
    u64 entry_point;
    u64 argc;
    u64 argv;
};

/**
 * @brief   Prepare execution environment(argc and argv for "main" function) and jump to main function
 * @note    RunElfParams and all pointers in it will leak;
 *          in the future they will be allocated in the process address space and cleaned up with it when process is done
 * @note    This function never returns; instead int 0x80 for sys_exit is called from within the elf itself
 */
[[noreturn]] void run_elf_in_current_addressspace(u64 arg) {
    RunElfParams* params = (RunElfParams*)arg;

    asm volatile(
            "mov %0, %%rdi       \n;"   // argc
            "mov %1, %%rsi       \n;"   // argv
            "jmp *%2             \n;"   // jump to "main" function address
            :
            : "g"(params->argc), "g"(params->argv), "g"(params->entry_point)
            : "%rsi", "%rdi"
    );

    __builtin_unreachable();
}

/**
 * @brief   Convert vector<string> into char*[]
 */
char** string_vec_to_argv(const vector<string>& src_vec) {
    u8 argc = src_vec.size() - 1;   // no first ("elfrun" cmd) item
    char** argv = new char* [argc];
    for (u8 i = 0; i < argc; i++) {
        const string& src = src_vec[i + 1];
        argv[i] = new char[src.length() + 1]; // +1 for null terminator
        memcpy(argv[i], src.c_str(), src.length() + 1); // +1 for null terminator
    }

    return argv;
}

char* elfrun::elf_physical_addr = nullptr;
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

    // load elf file data
    u32 size = e.get_size();
    char* buff = new char[size];
    e.read(buff, size);

    // prepare address space for the process; since currently no multiple elfs can be run at the same time, always reuse the same chunk of memory
    // notice that task stack pointer is still in kernel address space, same to main() argv
    // this will change as the kernel develops
    const size_t ELF_REQUIRED_MEM = 1024*1024*10;  // 10 MB should suffice for our tiny elfs
    if (!elf_physical_addr)
        elf_physical_addr = (char*)MemoryManager::instance().phys_alloc(ELF_REQUIRED_MEM);

    // create and switch to elf address space. since kernel space is mapped as well, we can continue executing kernel code
    hardware::PageTables::map_elf_address_space_at(elf_physical_addr, ELF_REQUIRED_MEM);

    // prepare elf run environment
    RunElfParams* run_env = new RunElfParams;
    run_env->entry_point = Elf64::load_into_current_addressspace(buff);
    run_env->argc = (u64)env->cmd_args.size() - 1; // no "elfrun" command
    run_env->argv = (u64)string_vec_to_argv(env->cmd_args);

    // run elf with environment pointer as an arg
    auto task = std::make_shared<Task>(run_elf_in_current_addressspace, name, (u64)run_env, true);
    TaskManager& task_manager = TaskManager::instance();
    task_manager.add_task(task);

    // wait until elf finishes and restore original kernel address space mapping
    task->wait_until_finished();
    hardware::PageTables::remap_kernel_higher_half();

    // free elf file data
    delete[] buff;
}

} /* namespace cmds */
