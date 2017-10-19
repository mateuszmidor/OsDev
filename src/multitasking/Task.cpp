/**
 *   @file: Task.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Task.h"
#include "PageTables.h"
#include "HigherHalf.h"
#include "VfsManager.h"
#include "KernelLog.h"

using namespace hardware;
using namespace filesystem;
namespace multitasking {


Task::Task() :
        entrypoint(nullptr), name("newtask"), arg1(0), arg2(0), is_user_space(false), pml4_phys_addr(0), cpu_state(0), stack_addr(0), stack_size(0), task_id(0) {
}

/**
 * Constructor.
 * Task stack is constructed as follows:
   0|FREE STACK|CpuState|TaskEpilogue|STACK_MAX
                        ^
                  here is rsp when first time jumping from interrupt to task function. So on return the function takes ret addr from TaskEpilogue.

  @param    entrypoint      Task main function address
  @param    arg1, arg2      Task main function param 1 and 2
  @param    pml4_phys_addr  Page tables root (PHYSICAL ADDRESS). Defines task address space.
                            If != 0, make sure the entrypoint and stack will be accessible from this address space
  @param    stack_addr      Virtual address of task stack
  @param    stack_size      Task stack size in bytes
*/
Task::Task(TaskEntryPoint2 entrypoint, const char name[], u64 arg1, u64 arg2, bool user_space, u64 pml4_phys_addr, u64 stack_addr, u64 stack_size) :
        entrypoint(entrypoint), name(name), arg1(arg1), arg2(arg2), is_user_space(user_space), pml4_phys_addr(pml4_phys_addr), cpu_state(0), task_id(0) {

    // create default task stack (mostly for kernel tasks)
    if (stack_addr == 0) {
        this->stack_addr = (u64)new char[DEFAULT_STACK_SIZE];
        this->stack_size = DEFAULT_STACK_SIZE;
    }
    // use provided stack (mostly for user tasks)
    else {
        this->stack_addr = stack_addr;
        this->stack_size = stack_size;
    }
}

/**
 * @brief   Setup cpu state and return address on the task stack befor running the task
 * @param   exitpoint Address of a function that the task should return to upon exit
 */
void Task::prepare(u32 tid, TaskExitPoint exitpoint) {
    const u64 STACK_END = (u64)stack_addr + stack_size;

    if (pml4_phys_addr == 0)
        pml4_phys_addr = PageTables::get_kernel_pml4_phys_addr(); // use kernel address space

    // prepare task epilogue ie. where to return from task function
    TaskEpilogue* task_epilogue = (TaskEpilogue*)(STACK_END - sizeof(TaskEpilogue));
    new (task_epilogue) TaskEpilogue {(u64)exitpoint};

    // prepare task cpu state to setup cpu register with
    cpu_state = (CpuState*)(STACK_END - sizeof(CpuState) - sizeof(TaskEpilogue));
    new (cpu_state) CpuState {(u64)entrypoint, (u64)task_epilogue, arg1, arg2, is_user_space, pml4_phys_addr};

    task_id = tid;
}

/**
 * @brief   Get file with given absolute "name", associate file descriptor to it and return the descriptor
 * @return  Descriptor number on success, -1 otherwise
 */
s32 Task::open_file(const char name[]) {
    VfsEntryPtr entry;
    for (u32 i = 5; i < files.size(); i++)
        if (!files[i]) {
            if ((entry = VfsManager::instance().get_entry(name)) && (!entry->is_directory())) {
                files[i] = entry;
                return i;
            } else
                return -1; // cant get such file
        }
    return -1; // open file limit reached
}

/**
 * @brief   Close file associated with "fd" file descriptor
 * @return  0 if valid "fd" provided, -1 otherwise
 */
s32 Task::close_file(u32 fd) {
    if (fd >= files.size())
        return -1;

    if (!files[fd])
        return -1;

    files[fd].reset();
    return 0;
}

s64 Task::read_file(u32 fd, void *buf, u64 count) {
    if (fd >= files.size())
        return 0;

    if (!files[fd])
        return 0;

    return files[fd]->read(buf, count);
}

void Task::idle() {
    while (true)
        yield();
}

void Task::yield() {
    asm volatile("hlt");
}

void Task::exit(u64 result_code) {
    asm volatile("int $0x80" : : "a"(1), "b"(result_code));
}

} /* namespace multitasking */
