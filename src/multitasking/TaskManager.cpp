/**
 *   @file: TaskManager.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskManager.h"

using namespace hardware;

namespace multitasking {


TaskManager TaskManager::_instance;

TaskManager& TaskManager::instance() {
    return _instance;
}

/**
 * Constructor.
 * Task stack is constructed as follows:
   0|FREE STACK|CpuState|TaskEpilogue|STACK_MAX
                        ^
                  here is rsp when first time jumping from interrupt to task function
*/
Task::Task(TaskEntryPoint entrypoint, kstd::string name, u64 arg) :
        entrypoint(entrypoint), name(name), arg(arg) {
    reset();
}

/**
 * @brief   Setup cpu state and return address on the task stack befor running the task
 */
void Task::reset() {
    const u64 STACK_END = (u64)stack + sizeof(stack);

    // prepare task epilogue ie. where to return from task function
    TaskEpilogue* task_epilogue = (TaskEpilogue*)(STACK_END - sizeof(TaskEpilogue));
    new (task_epilogue) TaskEpilogue {(u64)TaskManager::on_task_finished};

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

void TaskManager::on_task_finished() {
    Task::exit();
}

bool TaskManager::add_task(TaskPtr task) {
    if (num_tasks == tasks.size())
        return false;

    task->reset();

    tasks[num_tasks++] = task;
    return true;
}

TaskPtr TaskManager::get_current_task() const {
    return tasks[current_task];
}

u16 TaskManager::get_num_tasks() const {
    return num_tasks;
}

CpuState* TaskManager::schedule(CpuState* cpu_state) {
    if (num_tasks == 0)
        return cpu_state;

    if (current_task >= 0)
        tasks[current_task]->cpu_state = cpu_state;

    current_task = (current_task + 1) % num_tasks;
    return tasks[current_task]->cpu_state;
}

hardware::CpuState* TaskManager::kill_current_task() {
    // mark task as terminated
    tasks[current_task]->is_terminated = true;

    // remove current task from the list
    for (u16 i = current_task; i < num_tasks -1; i++)
        tasks[i] = tasks[i+1];
    num_tasks--;

    // return next task to switch to
    current_task = (current_task) % num_tasks;
    return tasks[current_task]->cpu_state;
}
} // namespace multitasking {
