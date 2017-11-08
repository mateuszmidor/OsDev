/**
 *   @file: TaskList.h
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_TASKLIST_H_
#define KERNEL_MULTITASKING_TASKLIST_H_

#include "Task.h"

namespace multitasking {

/**
 * @brief   Iterator interface for TaskList
 */
class TaskIterator  {
public:
    TaskIterator(Task* x) : current(x) {}

    TaskIterator(const TaskIterator& it) : current(it.current) {}

    TaskIterator& operator++() {
        if (current)
            current = current->next;
        return *this;
    }

    TaskIterator operator++(int) {
        TaskIterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const TaskIterator& rhs) const {
        return current == rhs.current;
    }

    bool operator!=(const TaskIterator& rhs) const {
        return current != rhs.current;
    }

    Task& operator*() {
        return *current;
    }

private:
    Task*   current = nullptr;
};


/**
 * @brief   This class represents a list of Tasks to hold the running/waiting tasks
 */
class TaskList {
public:
    Task* front();
    const Task* front() const;

    void push_front(Task* t);
    Task* pop_front();

    void remove(Task* t);

    bool contains(const Task* t) const;
    bool contains(u32 task_id) const;
    u32 count() const;

    // iterator interface
    const TaskIterator begin() const;
    const TaskIterator end() const;

private:
    Task*   m_head    = nullptr;
    u32     m_count   = 0;
};

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_TASKLIST_H_ */
