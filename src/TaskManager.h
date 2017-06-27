/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include <array>
#include "CpuState.h"

using TaskEntryPoint = void (*)();

struct Task {
    Task(TaskEntryPoint entrypoint);
    u8  stack[4096];
    cpu::CpuState* cpu_state;
};

class TaskManager {
public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    bool add_task(Task* task);
    cpu::CpuState* schedule(cpu::CpuState* cpu_state);

private:
    TaskManager() {}
    static TaskManager _instance;

    std::array<Task*, 256> tasks;
    u16 num_tasks       =  0;
    s16 current_task    = -1;
};

#endif /* SRC_TASKMANAGER_H_ */
