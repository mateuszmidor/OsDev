/**
 *   @file: TaskList.h
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_TASKLIST_H_
#define KERNEL_MULTITASKING_TASKLIST_H_

#include "types.h"

namespace multitasking {

// forward declaration
class Task;

/**
 * @brief   Iterator interface for TaskList
 */
class TaskIterator  {
public:
    TaskIterator(Task* x);
    TaskIterator(const TaskIterator& it);
    TaskIterator& operator++();
    TaskIterator operator++(int);
    bool operator==(const TaskIterator& rhs) const;
    bool operator!=(const TaskIterator& rhs) const;
    Task& operator*();

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
    Task* get_by_tid(u32 task_id);

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
