/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include <array>
#include "kstd.h"
#include "Task.h"

namespace multitasking {

class TaskManager {
    static constexpr u16 MAX_TASKS = 32;

public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    static void on_task_finished();
    u32 add_task(const Task& task);
    Task& get_current_task();
    const std::array<Task, MAX_TASKS>& get_tasks() const;
    u16 get_num_tasks() const;
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();
    void wait(u32 task_id) const;

private:
    TaskManager() {}
    hardware::CpuState* pick_next_task_and_load_address_space();
    void release_address_space(Task& task);

    static TaskManager _instance;

    std::array<Task, MAX_TASKS> tasks;
    u16 num_tasks       =  0;
    s16 current_task    = -1;
    u32 current_task_id =  1;
};

}

#endif /* SRC_TASKMANAGER_H_ */
