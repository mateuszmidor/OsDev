/**
 *   @file: TaskManager.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskManager.h"

using namespace cpu;

TaskManager TaskManager::_instance;

TaskManager& TaskManager::instance() {
    return _instance;
}

Task::Task(TaskEntryPoint entrypoint, kstd::string name) : name(name) {
    void* addr = (stack + sizeof(stack) - sizeof(CpuState)); // cpu state is stored at end of the stack
    cpu_state = new (addr) CpuState((u64)entrypoint);
}

bool TaskManager::add_task(Task* task) {
    if (num_tasks == tasks.size())
        return false;

    tasks[num_tasks++] = task;
    return true;
}

Task const* TaskManager::get_current_task() const {
    return tasks[current_task];
}

CpuState* TaskManager::schedule(CpuState* cpu_state) {
    if (num_tasks == 0)
        return cpu_state;

    if (current_task >= 0)
        tasks[current_task]->cpu_state = cpu_state;

    current_task = (current_task + 1) % num_tasks;
    return tasks[current_task]->cpu_state;
}

cpu::CpuState* TaskManager::kill_current_task() {
    // remove current task from the list
    for (auto i = current_task; i < num_tasks -1; i++)
        tasks[i] = tasks[i+1];
    num_tasks--;

    // return next task
    current_task = (current_task) % num_tasks;
    return tasks[current_task]->cpu_state;
}
