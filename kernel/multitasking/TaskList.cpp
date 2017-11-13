/**
 *   @file: TaskList.cpp
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#include "TaskList.h"
#include "Task.h"

namespace multitasking {

/**
 * @brief   Check if list contains item
 */
bool TaskList::contains(u32 task_id) const {
    for (const Task* t : *this)
        if (t->task_id == task_id)
            return true;

    return false;
}

/**
 * @brief   Get task of given tid
 */
Task* TaskList::get_by_tid(u32 task_id) {
    for (Task* t : *this)
        if (t->task_id == task_id)
            return t;

    return nullptr;
}

} /* namespace multitasking */
