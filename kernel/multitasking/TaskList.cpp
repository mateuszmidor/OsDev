/**
 *   @file: TaskList.cpp
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#include "TaskList.h"

namespace multitasking {

/**
 * @brief   Get first list item
 */
Task* TaskList::front() {
    return m_head;
}

/**
 * @brief   Get first list item
 */
const Task* TaskList::front() const {
    return m_head;
}

/**
 * @brief   Check if list contains item
 */
bool TaskList::contains(const Task* t) const {
    const Task* curr = m_head;

    while (curr)
        if (curr == t)
            return true;
        else
            curr = curr->next;

    return false;
}

/**
 * @brief   Check if list contains item
 */
bool TaskList::contains(u32 task_id) const {
    const Task* curr = m_head;

    while (curr)
        if (curr->task_id == task_id)
            return true;
        else
            curr = curr->next;

    return false;
}

/**
 * @brief   Insert item in front of the list
 */
void TaskList::attach_front(Task* t) {
    if (!t)
        return;

    t->next = m_head;
    m_head = t;
    m_count++;
}

/**
 * @brief   Remove item identified by pointer "t" from the list
 */
void TaskList::detach(Task* t) {
    // case 1. removing head item
    if (t == m_head) {
        Task* second = m_head->next;
        m_head->next = nullptr;
        m_head = second;
        m_count--;
        return;
    }

    // case 2. removing middle or tail item
    Task* prev = m_head;
    Task* curr = m_head->next;

    while (curr) {
        if (curr == t) {
            prev->next = curr->next;
            curr->next = nullptr;
            m_count--;
            return;
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }
}

/**
 * @brief   Move item to another list
 */
void TaskList::move_to(Task* t, TaskList& dst) {
    detach(t);
    dst.attach_front(t);
}

/**
 * @brief   Get number of items on list
 */
u32 TaskList::count() const {
    return m_count;
}

/**
 * @brief   Iterator interface
 */
const TaskIterator TaskList::begin() const {
    return TaskIterator(m_head);
}

/**
 * @brief   Iterator interface
 */
const TaskIterator TaskList::end() const {
    return TaskIterator(nullptr);
}

} /* namespace multitasking */
