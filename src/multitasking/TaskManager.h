/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include <array>
#include <memory>
#include "kstd.h"
#include "Task.h"

namespace multitasking {

class TaskManager {
public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    static void on_task_finished();
    bool add_task(TaskPtr task);
    TaskPtr get_current_task() const;
    u16 get_num_tasks() const;
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();

private:
    TaskManager() {}
    static TaskManager _instance;

    std::array<TaskPtr, 256> tasks;
    u16 num_tasks       =  0;
    s16 current_task    = -1;
};

}

#endif /* SRC_TASKMANAGER_H_ */
