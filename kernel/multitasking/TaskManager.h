/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include "kstd.h"
#include "Task.h"
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
    void replace_current_task(const Task& task);
    Task& get_current_task();
    const TaskList& get_tasks() const;
    u16 get_num_tasks() const;
    hardware::CpuState* sleep_current_task(hardware::CpuState* cpu_state, u64 millis);
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();
    bool wait(u32 task_id);
    void dequeue_current_task(TaskList& list);
    void enqueue_task_back(Task* task);

private:
    TaskManager() {}
    hardware::CpuState* pick_next_task_and_load_address_space();
    void close_files(Task& task);
    void release_address_space(Task& task);
    Task get_boot_task() const;
    static TaskManager _instance;

    Task                            boot_task       = get_boot_task();   // represents "kmain" boot task
    TaskList                        running_queue;                          // list of tasks that are ready to be run
    kstd::ListIterator<Task*>       current_task_it = running_queue.end();  // task that is currently running
    kstd::ListIterator<Task*>       next_task_it    = running_queue.end();  // next task to be run
    u32                             next_task_id    =  1;                   // id to assign to the next task while adding
};

}

#endif /* SRC_TASKMANAGER_H_ */
