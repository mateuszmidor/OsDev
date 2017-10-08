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
 * @brief   When a task finishes its work, it jumps here for the TaskManeger to clear it from the list
 */
void TaskManager::on_task_finished() {
    Task::exit();
}

/**
 * @brief   Add new task to the list of scheduler tasks
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 */
bool TaskManager::add_task(TaskPtr task) {
    if (num_tasks == tasks.size())
        return false;

    task->prepare(TaskManager::on_task_finished);

    tasks[num_tasks++] = task;
    return true;
}

TaskPtr TaskManager::get_current_task() const {
    return tasks[current_task];
}

const std::array<TaskPtr, 256>& TaskManager::get_tasks() const {
    return tasks;
}

u16 TaskManager::get_num_tasks() const {
    return num_tasks;
}

/**
 * @brief   Task scheduling routine. Simple round robin is used for now
 * @param   cpu_state Current task cpu state.
 * @note    cpu_state IS LOCATED ON THE KERNEL STACK AND NEEDS TO BE COPIED TO THE TASK-SPECIFIC LOCATION IF SWITCHING FROM RING 3 (USER SPACE)!!!
 *          This can be improved by storing cpu_state directly in task-specific location or using per-task kernel stack. To be done later.
 * @note    There must be always at least 1 ("idle") task
 * @note    This is supposed to be run from interrupt handling context
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 */
CpuState* TaskManager::schedule(CpuState* cpu_state) {
    if (num_tasks == 0)
        return cpu_state;

    if (current_task >= 0) {
        if (tasks[current_task]->is_user_space) {
            tasks[current_task]->cpu_state = (CpuState*) (cpu_state->rsp - sizeof(CpuState)); // allocate cpu state on the current task stack
            *(tasks[current_task]->cpu_state) = *cpu_state; // copy cpu state from kernel stack to task stack
        }
        else
            tasks[current_task]->cpu_state = cpu_state;     // cpu state is already allocated and stored on task stack, just remember the pointer
    }

    current_task = (current_task + 1) % num_tasks;
    u64 pml4_physical_address = tasks[current_task]->pml4_phys_addr;
    asm volatile (
            "mov %%rax, %%cr3       ;"
            :
            : "a"(pml4_physical_address)
            :
    );
    return tasks[current_task]->cpu_state;
}

/**
 * @brief   Remove current task from the list and return new task to be run
 * @note    This is supposed to be run from interrupt handling context
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 */
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
