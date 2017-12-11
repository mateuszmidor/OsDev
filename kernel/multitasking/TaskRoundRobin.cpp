/**
 *   @file: TaskRoundRobin.cpp
 *
 *   @date: Dec 8, 2017
 * @author: Mateusz Midor
 */

#include "TaskRoundRobin.h"
#include "TaskFactory.h"

namespace multitasking {

bool TaskRoundRobin::add(Task* t) {
    if (running_queue.count() == MAX_TASKS)
        return false;

    running_queue.push_front(t);
    return true;
}

void TaskRoundRobin::remove(Task* t) {
    auto it = running_queue.find(t);
    if (it == running_queue.end())
        return;

    // remove task from the queue
    running_queue.remove(it);

    // invalidate next task iterator, if needed
    if ((next_task_it != running_queue.end()) && (*next_task_it == t))
        next_task_it = running_queue.end();
}

Task* TaskRoundRobin::get_by_tid(u32 task_id) {
    return running_queue.get_by_tid(task_id);
}

Task* TaskRoundRobin::get_current_task() {
    return current_task;
}

Task* TaskRoundRobin::pick_next_task() {
    if (next_task_it == running_queue.end())
        next_task_it = running_queue.begin();

    current_task = *next_task_it;
    next_task_it = next_task_it.get_next();
    return current_task;
}

const TaskList& TaskRoundRobin::get_task_list() const {
    return running_queue;
}

u32 TaskRoundRobin::count() const {
    return running_queue.count();
}




//
///**
// * @brief   Take the current task out of the running queue and put it on "list"
// * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
// */
//void TaskRoundRobin::dequeue_current_task(TaskList& list) {
//    list.push_front(current_task);
//    running_queue.remove(current_task);
//}
//
//Task* TaskRoundRobin::get_by_tid(u32 task_id) {
//    running_queue.get_by_tid(task_id);
//}
//
//kstd::ListIterator<Task*> TaskRoundRobin::find(const Task* t) {
//    return running_queue.find(t);
//}
///**
// * @brief   Get dummy boot task, just so there is always some get_current_task() result
// */
//Task TaskRoundRobin::get_boot_task() const {
//    return TaskFactory::make_boot_stub_task();
//}

} /* namespace multitasking */
