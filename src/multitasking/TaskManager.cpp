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

void TaskManager::on_task_finished() {
    Task::exit();
}

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
