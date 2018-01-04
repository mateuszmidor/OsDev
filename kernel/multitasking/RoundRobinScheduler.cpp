/**
 *   @file: RoundRobinScheduler.cpp
 *
 *   @date: Dec 8, 2017
 * @author: Mateusz Midor
 */

#include "RoundRobinScheduler.h"
#include "Task.h"

namespace multitasking {

/**
 * @brief   Add new task to the scheduler list
 * @return  True on success, False otherwise
 */
bool RoundRobinScheduler::add(Task* t) {
    if (tasks.count() == MAX_TASKS)
        return false;

    tasks.push_front(t);
    return true;
}

/**
 * @brief   Remove task from scheduler
 */
void RoundRobinScheduler::remove(Task* t) {
    auto it = tasks.find(t);
    if (it == tasks.end())
        return;

    // remove task from the queue
    tasks.remove(it);

    // invalidate next task iterator, if needed
    if ((next_task_it != tasks.end()) && (*next_task_it == t))
        next_task_it = tasks.end();
}

/**
 * @brief   Find a task by its task_id
 * @return  Task pointer on success, nullptr otherwise
 */
Task* RoundRobinScheduler::get_by_tid(u32 task_id) {
    return tasks.get_by_tid(task_id);
}

/**
 * @brief   Get pointer to the task that is currently being executed
 */
Task* RoundRobinScheduler::get_current_task() {
    return current_task;
}

/**
 * @brief   Choose and return next task to be executed
 */
Task* RoundRobinScheduler::pick_next_task() {
    do {
        if (next_task_it == tasks.end())
            next_task_it = tasks.begin();

        current_task = *next_task_it;
        next_task_it = next_task_it.get_next();
    } while (current_task->state != TaskState::RUNNING);
    return current_task;
}

/**
 * @brief   Get unmodifiable list of scheduler tasks
 */
const TaskList& RoundRobinScheduler::get_task_list() const {
    return tasks;
}

/**
 * @brief   Get scheduler task count
 */
u32 RoundRobinScheduler::count() const {
    return tasks.count();
}


} /* namespace multitasking */
