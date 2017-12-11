/**
 *   @file: TaskRoundRobin.h
 *
 *   @date: Dec 8, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_TASKROUNDROBIN_H_
#define KERNEL_MULTITASKING_TASKROUNDROBIN_H_

#include "TaskList.h"

namespace multitasking {

/**
 * @brief   This class provides round-robin scheduling of tasks
 */
class TaskRoundRobin {
    static constexpr u32 MAX_TASKS  {32};

public:
    bool add(Task* task);
    void remove(Task* t);
    Task* get_by_tid(u32 task_id);
    Task* get_current_task();
    Task* pick_next_task();
    const TaskList& get_task_list() const;
    u32 count() const;

private:
    Task get_boot_task() const;

    TaskList                    running_queue;
    Task*                       current_task    {nullptr};
    kstd::ListIterator<Task*>   next_task_it    {running_queue.end()};  // next task to be run

};

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_TASKROUNDROBIN_H_ */
