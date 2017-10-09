/**
 *   @file: Task.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Task.h"
#include "PageTables.h"
#include "MemoryManager.h"

using namespace hardware;
namespace multitasking {



/**
 * Constructor.
 * Task stack is constructed as follows:
   0|FREE STACK|CpuState|TaskEpilogue|STACK_MAX
                        ^
                  here is rsp when first time jumping from interrupt to task function
*/
Task::Task(TaskEntryPoint entrypoint, kstd::string name, u64 arg, bool user_space, u64 pml4_phys_addr, u64 stack_addr, u64 stack_size) :
        entrypoint(entrypoint), name(name), arg(arg), is_user_space(user_space), pml4_phys_addr(pml4_phys_addr), cpu_state(0) {
    if (stack_addr != 0) {
        this->stack_addr = stack_addr;
        this->stack_size = stack_size;
    }
    else {
        u64 DEFAULT_STACK_SIZE = 2 * 4096;
        this->stack_addr = (u64)memory::MemoryManager::instance().virt_alloc(DEFAULT_STACK_SIZE);
        this->stack_size = DEFAULT_STACK_SIZE;
    }
}

/**
 * @brief   Setup cpu state and return address on the task stack befor running the task
 * @param   exitpoint Address of a function that the task should return to upon exit
 */
void Task::prepare(TaskExitPoint exitpoint) {
    const u64 STACK_END = (u64)stack_addr + stack_size;

    if (pml4_phys_addr == 0)
        pml4_phys_addr = PageTables::get_kernel_pml4_phys_addr(); // use kernel address space

    // prepare task epilogue ie. where to return from task function
    TaskEpilogue* task_epilogue = (TaskEpilogue*)(STACK_END - sizeof(TaskEpilogue));
    new (task_epilogue) TaskEpilogue {(u64)exitpoint};

    // prepare task cpu state to setup cpu register with
    cpu_state = (CpuState*)(STACK_END - sizeof(CpuState) - sizeof(TaskEpilogue));
    new (cpu_state) CpuState {(u64)entrypoint, (u64)task_epilogue, arg, is_user_space, pml4_phys_addr};

    is_terminated = false;
}

/**
 * is_terminated is set by TaskManager::kill_current_task
 */
void Task::wait_until_finished() {
    while (!is_terminated)
        Task::yield();
}

void Task::idle(u64 arg) {
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
