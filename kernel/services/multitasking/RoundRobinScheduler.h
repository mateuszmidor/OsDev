/**
 *   @file: RoundRobinScheduler.h
 *
 *   @date: Dec 8, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_ROUNDROBINSCHEDULER_H_
#define KERNEL_MULTITASKING_ROUNDROBINSCHEDULER_H_

#include "TaskList.h"

namespace multitasking {

/**
 * @brief   This class provides a round-robin task scheduler.
 */
class RoundRobinScheduler {
    static constexpr u32 MAX_TASKS  {32};   // 32 is arbitrarily chosen, can put here more

public:
    RoundRobinScheduler(Task* boot_task) : current_task(boot_task) {}
    void set_idle_task(Task* task);
    bool add(Task* task);
    void remove(Task* t);
    bool is_valid_task(Task* task) const;
    Task* get_by_tid(u32 task_id);
    Task* get_current_task();
    Task* pick_next_task();
    const TaskList& get_task_list() const;
    u32 count() const;

private:
    Task* find_next_runnable_task();

    TaskList                    tasks;
    Task*                       idle            {nullptr};
    Task*                       current_task;
    kstd::ListIterator<Task*>   next_task_it    {tasks.end()};

};

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_ROUNDROBINSCHEDULER_H_ */
