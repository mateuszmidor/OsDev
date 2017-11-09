/**
 *   @file: TaskList.cpp
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#include "TaskList.h"
#include "Task.h"

namespace multitasking {


/*******************************************************************************************
 * class   TaskIterator
 *******************************************************************************************/
TaskIterator::TaskIterator(Task* x) : current(x) {}

TaskIterator::TaskIterator(const TaskIterator& it) : current(it.current) {}

TaskIterator& TaskIterator::operator++() {
    if (current)
        current = current->next;
    return *this;
}

TaskIterator TaskIterator::operator++(int) {
    TaskIterator tmp(*this);
    operator++();
    return tmp;
}

bool TaskIterator::operator==(const TaskIterator& rhs) const {
    return current == rhs.current;
}

bool TaskIterator::operator!=(const TaskIterator& rhs) const {
    return current != rhs.current;
}

Task& TaskIterator::operator*() {
    return *current;
}



/*******************************************************************************************
 * class   TaskList
 *******************************************************************************************/

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
 * @brief   Get task of given tid
 */
Task* TaskList::get_by_tid(u32 task_id) {
    Task* t = m_head;
    while (t) {
        if (t->task_id == task_id)
            return t;
        else
            t = t->next;
    }

    return nullptr;
}

/**
 * @brief   Insert item in front of the list
 */
void TaskList::push_front(Task* t) {
    if (!t)
        return;

    t->next = m_head;
    m_head = t;
    m_count++;
}

/**
 * @brief   Take out first item
 */
Task* TaskList::pop_front() {
    Task* t = m_head;
    remove(m_head);
    return t;
}



/**
 * @brief   Remove item identified by pointer "t" from the list
 */
void TaskList::remove(Task* t) {
    // nothing to remove
    if (!m_head)
        return;

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
