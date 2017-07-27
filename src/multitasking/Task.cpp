/**
 *   @file: Task.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Task.h"

using namespace hardware;
namespace multitasking {



/**
 * Constructor.
 * Task stack is constructed as follows:
   0|FREE STACK|CpuState|TaskEpilogue|STACK_MAX
                        ^
                  here is rsp when first time jumping from interrupt to task function
*/
Task::Task(TaskEntryPoint entrypoint, kstd::string name, u64 arg) :
        entrypoint(entrypoint), name(name), arg(arg), cpu_state(0) {
}

/**
 * @brief   Setup cpu state and return address on the task stack befor running the task
 * @param   exitpoint Address of a function that the task should return to upon exit
 */
void Task::prepare(TaskExitPoint exitpoint) {
    const u64 STACK_END = (u64)stack + sizeof(stack);

    // prepare task epilogue ie. where to return from task function
    TaskEpilogue* task_epilogue = (TaskEpilogue*)(STACK_END - sizeof(TaskEpilogue));
    new (task_epilogue) TaskEpilogue {(u64)exitpoint};

    // prepare task cpu state to setup cpu register with
    cpu_state = (CpuState*)(STACK_END - sizeof(CpuState) - sizeof(TaskEpilogue));
    new (cpu_state) CpuState {(u64)entrypoint, (u64)task_epilogue, arg};

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
    asm("hlt");
}

void Task::exit() {
    // int 15 is handled by TaskExitHandler that simply calls TaskManager::kill_current_task()
    asm("int $15");
}

} /* namespace multitasking */
