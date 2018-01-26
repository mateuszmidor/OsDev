/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include "Task.h"
#include "TaskList.h"
#include "RoundRobinScheduler.h"

namespace multitasking {

class TaskManager {
public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    void install_multitasking();
    u32 add_task(Task* task);
    void replace_current_task(Task* task);
    Task& get_current_task();
    const TaskList& get_tasks() const;
    hardware::CpuState* sleep_current_task(hardware::CpuState* cpu_state, u64 millis);
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();
    hardware::CpuState* kill_current_task_group();
    void mark_task_tree_to_terminate(Task* task);
    bool wait(u32 task_id);
    void block_current_task(TaskList& list);
    void unblock_tasks(TaskList& list);

private:
    static void on_task_finished();
    TaskManager() : boot_task(get_boot_task()), scheduler(&boot_task) {}
    void save_current_task_state(hardware::CpuState* cpu_state);
    hardware::CpuState* pick_next_task_and_load_address_space();
    void wakeup_waitings_and_delete_task(Task* task);
    Task get_boot_task() const;
    void kill_task(Task* current_task);

    static TaskManager _instance;

    Task                    boot_task;              // represents "kmain" boot task
    RoundRobinScheduler     scheduler;
    u32                     next_task_id    = 0;    // id to assign to the next task while adding
};

}

#endif /* SRC_TASKMANAGER_H_ */
