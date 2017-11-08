/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include "kstd.h"
#include "TaskList.h"

namespace multitasking {

class TaskManager {
    static constexpr u16 MAX_TASKS = 32;

public:
    static TaskManager& instance();
    static void on_task_finished();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    u32 add_task(const Task& task);
    Task& get_current_task();
    const TaskList& get_tasks() const;
    u16 get_num_tasks() const;
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();
    void wait(u32 task_id) const;
    void current_wait_on(TaskList& wait_list);

private:
    TaskManager() {}
    hardware::CpuState* pick_next_task_and_load_address_space();
    void close_files(Task& task);
    void release_address_space(Task& task);

    static TaskManager _instance;

    TaskList    tasks;
    Task*       current_task    = nullptr;
    u32         current_task_id =  1;
};

}

#endif /* SRC_TASKMANAGER_H_ */
