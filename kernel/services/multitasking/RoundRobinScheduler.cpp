/**
 *   @file: RoundRobinScheduler.cpp
 *
 *   @date: Dec 8, 2017
 * @author: Mateusz Midor
 */

#include "RoundRobinScheduler.h"
#include "Assert.h"
#include "Task.h"

namespace multitasking {

/**
 * @brief   Idle is the task that is picked when there is no other runnable task available
 */
void RoundRobinScheduler::set_idle_task(Task* task) {
    idle = task;
}

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
 * @brief   Check if "task" is a known task
 */
bool RoundRobinScheduler::is_valid_task(Task* task) const {
    return tasks.find(task) != tasks.end();
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
 * @brief   Choose and return next task to be executed.
 *          Can be the curr task if no other is eligible.
 *          Can be idle task if even curr is not eligible
 */
Task* RoundRobinScheduler::pick_next_task() {
    utils::phobos_assert(tasks.count() > 0, "RoundRobinScheduler::pick_next_task: no tasks to pick from");

    // find next task in runnable state or even itself if it's the only runnable one
    if (Task* t = find_next_runnable_task())
        return (current_task = t);

    // task eligible to run not found, do idle
    return (current_task = idle);
}

/**
 * @brief  Get next task in runnable state. Or even the current task if it's the only runnable task. Or nullptr if no runnable task available at all
 */
Task* RoundRobinScheduler::find_next_runnable_task() {
    for (size_t i = 0; i <= tasks.count(); i++) { // circular list
        if (next_task_it == tasks.end())
            next_task_it = tasks.begin();

        current_task = *next_task_it;
        next_task_it = next_task_it.get_next();
        if (current_task->state == TaskState::RUNNING)
            return current_task;
    }

    // no runnable task found
    return nullptr;
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
